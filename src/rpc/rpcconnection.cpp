/****************************************************************************

This file is part of the QtMediaHub project on http://www.gitorious.org.

Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation (qt-info@nokia.com)**

You may use this file under the terms of the BSD license as follows:

"Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."

****************************************************************************/

#include "rpcconnection.h"
#include <QMetaMethod>
#include <QTcpServer>
#include <QTcpSocket>
#include <netinet/in.h>

#include "qjson/src/parser.h"
#include "qjson/src/serializer.h"

RpcConnection::RpcConnection(RpcConnection::Mode mode, QObject *parent)
    : QObject(parent), m_mode(mode), m_server(0), m_socket(0)
{
    if (m_mode == Server) {
        listen(QHostAddress::Any, 1234);
    } else if (m_mode == Client) {
        connectToHost(QHostAddress::Any, 1234);
    }
}

void RpcConnection::registerObject(QObject *object)
{
    m_objects[object->objectName()] = object;
}

void RpcConnection::connectToHost(const QHostAddress &address, quint16 port)
{
    if (!m_socket)
        m_socket = new QTcpSocket(this);
    m_socket->connectToHost(address, port);
}

bool RpcConnection::waitForConnected(int msecs)
{
    return m_socket->waitForConnected(msecs);
}

bool RpcConnection::listen(const QHostAddress &address, quint16 port)
{
    if (!m_server) {
        m_server = new QTcpServer(this);
        connect(m_server, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
    }

    return m_server->listen(address, port);
}

void RpcConnection::handleNewConnection()
{
    // ## Handle multiple connections
    m_socket = m_server->nextPendingConnection();
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
    qDebug() << "Connected to client";
}

void RpcConnection::handleReadyRead()
{
    struct Header { int length; } header;
    m_socket->read((char *)&header, sizeof(header));
    header.length = ntohl(header.length);
    QByteArray msg = m_socket->read(header.length);
    qDebug() << "Received " << msg;

    bool ok;
    QJson::Parser parser;
    QVariantMap map = parser.parse(msg, &ok).toMap();
    if (!ok || map["jsonrpc"].toString() != "2.0") {
        qWarning() << "Malformatted JSON-RPC";
        return;
    }

    QString rpcMethod = map["method"].toString();
    int idx = rpcMethod.indexOf('.');
    QString objName = rpcMethod.mid(0, idx);
    QString method = rpcMethod.mid(idx+1) + QString::fromLatin1("()");
    QObject *object = m_objects.value(objName);
    if (!object) {
        qWarning() << "RPC Method " << msg << " not found";
        return;
    }

    idx = object->metaObject()->indexOfMethod(method.toLatin1());
    if (idx < 0) {
        qWarning() << "Object '" << objName << "' does not have method named " << method;
        return;
    }
    QMetaMethod mm = object->metaObject()->method(idx);
    if (mm.access() == QMetaMethod::Private || mm.methodType() == QMetaMethod::Signal) {
        qWarning() << "Method " << method << " is a signal or has private access";
        return;
    }
    // ## Fill with arguments from the message
    void *args[] = { };
    QMetaObject::metacall(object, QMetaObject::InvokeMetaMethod, idx, args);
}

bool RpcConnection::call(const QByteArray &method, const QVariant &arg1, const QVariant &arg2)
{
    struct Header { int length; } header;

    QVariantMap map;
    map.insert("jsonrpc", "2.0");
    map.insert("method", method);
    map.insert("params", QVariantList()); // ## FIXME: add params
    QJson::Serializer serializer;
    QByteArray jsonRpc = serializer.serialize(QVariant(map));

    header.length = htonl(jsonRpc.length());
    m_socket->write((const char *)&header, sizeof(header));
    m_socket->write(jsonRpc);
    m_socket->flush();

    return true;
}

