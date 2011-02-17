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

#ifndef RPCCONNECTION_H
#define RPCCONNECTION_H

#include <QObject>
#include <QHostAddress>
#include <QVariant>

class QTcpServer;
class QTcpSocket;

class RpcConnection : public QObject
{
    Q_OBJECT
public:
    enum Mode {
        Server,
        Client
    };
    RpcConnection(Mode mode, const QHostAddress &address, qint32 port, QObject *parent = 0);
    RpcConnection(Mode mode = Client, QObject *parent = 0);

    enum ErrorCode {
        ParseError = -32700,
        InvalidRequest = -32600,
        MethodNotFound = -32601,
        InvalidParameters = -32602,
        InternalError = -32603,
        ServerErrorBase = -32099,
        ServerErrorEnd = -32000
    };

    void registerObject(QObject *object);

    bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
    void connectToHost(const QHostAddress &address, quint16 port);
    bool waitForConnected(int msecs = 5000);

public slots:
    void connectToHost(const QString &address, quint16 port) { connectToHost(QHostAddress(address), port); }
    void disconnectFromHost();

    int call(const QByteArray &method, 
             const QVariant &arg0 = QVariant(), const QVariant &arg1 = QVariant(),
             const QVariant &arg2 = QVariant(), const QVariant &arg3 = QVariant(),
             const QVariant &arg4 = QVariant(), const QVariant &arg5 = QVariant(),
             const QVariant &arg6 = QVariant(), const QVariant &arg7 = QVariant(),
             const QVariant &arg8 = QVariant(), const QVariant &arg9 = QVariant());

    // ## This is a workaround for QML/4.7.0 which is unable to convert int to QVariant
    int call(const QByteArray &method, int arg0) { return call(method, QVariant(arg0)); }

signals:
    void clientConnected();
    void clientDisconnected();

private slots:
    void handleNewConnection();
    void handleReadyRead();

private:
    void init();

    void sendResponse(QTcpSocket *socket, const QString &id, const QVariant &result);
    void sendError(QTcpSocket *socket, const QString &id, int error, const QString &message, const QString &data = QString());

    void handleRpcCall(QTcpSocket *socket, const QVariantMap &map);
    void handleRpcResponse(const QVariantMap &map);
    void handleRpcError(const QVariantMap &map);

    Mode m_mode;
    QTcpServer *m_server;
    QTcpSocket *m_socket;
    QHash<QString, QObject *> m_objects;
    int m_id;
};

#endif // RPCCONNECTION_H

