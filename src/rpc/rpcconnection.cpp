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

#include "rpcconnection.h"
#include <QMetaMethod>
#include <QStringList>
#include <QVarLengthArray>
#include <QTcpServer>
#include <QTcpSocket>
#include <netinet/in.h>

#include "jsonparser/json.h"

RpcConnection::RpcConnection(RpcConnection::Mode mode, const QHostAddress &address, qint32 port, QObject *parent)
    : QObject(parent), m_mode(mode), m_server(0), m_socket(0), m_id(1)
{
    init();
    if (m_mode == Server) {
        listen(address, port);
    } else if (m_mode == Client) {
        connectToHost(address, port);
    }
}

RpcConnection::RpcConnection(RpcConnection::Mode mode, QObject *parent)
    : QObject(parent), m_mode(mode), m_server(0), m_socket(0), m_id(1)
{
    init();
}

void RpcConnection::init()
{
    if (m_mode == Client) {
        m_socket = new QTcpSocket(this);
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
        connect(m_socket, SIGNAL(connected()), this, SIGNAL(clientConnected()));
        connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(clientDisconnected()));
    } else {
        m_server = new QTcpServer(this);
        connect(m_server, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
    }
}

void RpcConnection::registerObject(QObject *object)
{
    if (object->objectName().startsWith("rpc.")) {
        qWarning() << "Method names starting with 'rpc.' are reserved by the specification";
        return;
    }
    m_objects[object->objectName()] = object;
}

void RpcConnection::unregisterObject(QObject *object)
{
    m_objects.remove(object->objectName());
}

void RpcConnection::connectToHost(const QHostAddress &address, quint16 port)
{
    Q_ASSERT(m_mode == Client);
    m_socket->connectToHost(address, port);
}

void RpcConnection::disconnectFromHost()
{
    Q_ASSERT(m_mode == Client);
    m_socket->disconnectFromHost();
}

bool RpcConnection::waitForConnected(int msecs)
{
    Q_ASSERT(m_mode == Client);
    return m_socket->waitForConnected(msecs);
}

bool RpcConnection::listen(const QHostAddress &address, quint16 port)
{
    Q_ASSERT(m_mode == Server);
    return m_server->listen(address, port);
}

void RpcConnection::handleNewConnection()
{
    QPointer<QTcpSocket> socket = m_server->nextPendingConnection();
    connect(socket, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    m_clients << socket;
    // qDebug() << "Connected to client";
}

void RpcConnection::sendError(QTcpSocket *socket, const QString &id, int error, const QString &message, const QString &data)
{
    QVariantMap responseMap;
    responseMap.insert("jsonrpc", "2.0");
    responseMap.insert("id", id);

    QVariantMap errorMap;
    errorMap.insert("code", error);
    errorMap.insert("message", message);
    errorMap.insert("data", data);
    responseMap.insert("error", errorMap);

    JsonWriter writer;
    writer.stringify(QVariant(responseMap));
    QByteArray jsonRpc = writer.result().toUtf8();

    struct Header { int length; } header;
    header.length = htonl(jsonRpc.length());
    socket->write((const char *)&header, sizeof(header));
    socket->write(jsonRpc);
    socket->flush();
}

void RpcConnection::handleReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    Q_ASSERT(socket);
    do {
        struct Header { int length; } header;
        socket->read((char *)&header, sizeof(header));
        header.length = ntohl(header.length);
        QByteArray msg = socket->read(header.length); // ## assumes we got complete message

        JsonReader reader;
        if (!reader.parse(msg)) {
            if (m_mode == Server)
                sendError(socket, "null", ParseError, "Not JSON", reader.errorString());
            qWarning() << "Not a JSON " << msg;
            return;
        }
        QVariantMap map = reader.result().toMap();
        if (map["jsonrpc"].toString() != "2.0") {
            if (m_mode == Server)
                sendError(socket, map["id"].toString(), InvalidRequest, "Version mismatch", "Expecting JSON-RPC 2.0");
            qWarning() << "Version mismatch, expecting 2.0 JSON-RPC";
            return;
        }

        if (map.contains("method")) {
            handleRpcCall(socket, map);
        } else if (map.contains("result")) {
            handleRpcResponse(map);
        } else if (map.contains("error")) {
            handleRpcError(map);
        } else {
            sendError(socket, map["id"].toString(), InvalidRequest, "Invalid request. Mot a method call, result or error");
            qWarning() << "No idea what to do with this message" << msg;
        }
    } while (socket->bytesAvailable() != 0);
}

static QByteArray stringifyParamTypes(const QVariantList &params)
{
    QByteArray ba;
    for (int i = 0; i < params.count(); i++) {
        if (i)
            ba.append(",");
        ba.append(params[i].typeName());
    }
    return ba;
}

void RpcConnection::handleRpcCall(QTcpSocket *socket, const QVariantMap &map)
{
    QVariantList params = map["params"].toList();

    QString rpcMethod = map["method"].toString();
    int idx = rpcMethod.indexOf('.');
    QString objName = rpcMethod.mid(0, idx);
    QString method = rpcMethod.mid(idx+1);
    QObject *object = m_objects.value(objName);
    if (!object) {
        sendError(socket, map["id"].toString(), MethodNotFound, "No such object");
        qWarning() << "RPC object " << rpcMethod << " not found";
        return;
    }

    QByteArray normalizedMethod = method.toLatin1() + "(" + stringifyParamTypes(params) + ")";
    idx = object->metaObject()->indexOfMethod(normalizedMethod);
    if (idx < 0) {
        sendError(socket, map["id"].toString(), MethodNotFound, "No such method");
        qWarning() << "Object '" << qPrintable(objName) << "' does not have method named " << method;
        return;
    }
    QMetaMethod mm = object->metaObject()->method(idx);
    if (mm.access() == QMetaMethod::Private || mm.methodType() == QMetaMethod::Signal) {
        sendError(socket, map["id"].toString(), MethodNotFound, "Method is private or is a signal");
        qWarning() << "Method " << method << " is a signal or has private access";
        return;
    }

    QVarLengthArray<void *, 10> args(params.count()+1);
    QVariant result;
    if (mm.typeName()) {
        result = QVariant(QVariant::nameToType(mm.typeName()), (void *)0);
        args[0] = result.data();
    } else {
        args[0] = 0;
    }

    for (int i = 0; i < params.count(); i++) {
        args[i+1] = params[i].data();
    }

    QMetaObject::metacall(object, QMetaObject::InvokeMetaMethod, idx, args.data());

    sendResponse(socket, map["id"].toString(), args[0] ? *static_cast<QVariant *>(args[0]) : QVariant());
}

void RpcConnection::sendResponse(QTcpSocket *socket, const QString &id, const QVariant &result)
{
    QVariantMap responseMap;
    responseMap.insert("jsonrpc", "2.0");
    responseMap.insert("result", result);
    responseMap.insert("id", id);
    JsonWriter writer;
    writer.stringify(responseMap);
    QByteArray jsonRpc = writer.result().toUtf8();

    struct Header { int length; } header;
    header.length = htonl(jsonRpc.length());
    socket->write((const char *)&header, sizeof(header));
    socket->write(jsonRpc);
    socket->flush();
}

void RpcConnection::handleRpcResponse(const QVariantMap &map)
{
    Q_UNUSED(map);
    // qDebug() << "Response" << map;
}

void RpcConnection::handleRpcError(const QVariantMap &map)
{
    qDebug() << "ERROR" << map;
}

int RpcConnection::call(const QByteArray &method, const QVariant &arg0, const QVariant &arg1,
                        const QVariant &arg2, const QVariant &arg3, const QVariant &arg4, const QVariant &arg5,
                        const QVariant &arg6, const QVariant &arg7, const QVariant &arg8, const QVariant &arg9)
{
        struct Header { int length; } header;

        QVariantList params;
        params << arg0 << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9;
        int i;
        for (i = 0; i < params.count(); i++) {
            if (params[i].isNull())
                break;
        }
        params = params.mid(0, i);

        QVariantMap map;
        map.insert("jsonrpc", "2.0");
        map.insert("method", method);
        map.insert("params", params);
        map.insert("id", m_id++);
        JsonWriter writer;
        writer.stringify(map);
        QByteArray jsonRpc = writer.result().toUtf8();

    if (m_mode == Client) {
        header.length = htonl(jsonRpc.length());
        m_socket->write((const char *)&header, sizeof(header));
        m_socket->write(jsonRpc);
        m_socket->flush();
    } else {
        foreach (QPointer<QTcpSocket> socket, m_clients) {
            if (socket.isNull()) {
                m_clients.removeOne(socket);
                continue;
            }
            header.length = htonl(jsonRpc.length());
            socket->write((const char *)&header, sizeof(header));
            socket->write(jsonRpc);
            socket->flush();
        }
    }

    return m_id;
}

