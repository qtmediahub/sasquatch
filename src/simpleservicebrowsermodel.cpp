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

#include <QUdpSocket>
#include <QByteArray>
#include <QHostAddress>
#include <QHostInfo>
#include "simpleservicebrowsermodel.h"

SimpleServiceBrowserModel::SimpleServiceBrowserModel(QObject *parent) :
    QAbstractListModel(parent), port(52109), identifier("QtMediaHub:"),
    ping("Ping:"), pong("Pong:")
{
    QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();
    roleNames[Qt::DisplayRole] = "display";
    roleNames[AddressRole] = "address";
    roleNames[PortRole] = "port";
    setRoleNames(roleNames);

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(port, QUdpSocket::ShareAddress);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processDatagrams()));

    QByteArray datagram = identifier + ping + QHostInfo::localHostName().toAscii();
    udpSocket->writeDatagram(datagram.data(), datagram.size(), QHostAddress::Broadcast, port);
}

void SimpleServiceBrowserModel::processDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray received;
        QHostAddress address;
        received.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(received.data(), received.size(), &address);

        if (received.startsWith(identifier)) {
            received.remove(0, identifier.size());
            if (received.startsWith(ping) && received.size() > ping.size()
                && received.indexOf(QHostInfo::localHostName().toAscii(), ping.size()) < 0) {
                addDevice(received.mid(ping.size(), 30), address.toString());
                QByteArray datagram = identifier + pong + QHostInfo::localHostName().toAscii();
                udpSocket->writeDatagram(datagram.data(), datagram.size(), address, port);
            } else if (received.startsWith(pong) && received.size() > pong.size()) {
                addDevice(received.mid(pong.size(), 30), address.toString());
            }
        }
    }
}

void SimpleServiceBrowserModel::addDevice(const QString &name, const QString &ip)
{
    if (count() < 33) {
        emit beginResetModel();
        devices.insert(name, ip);
        emit endResetModel();
    }
}

QVariant SimpleServiceBrowserModel::data(const QModelIndex &index, int role) const
{
    QVariant rv;

    if (!index.isValid())
        return rv;

    if (0 <= index.row() && index.row() < devices.size()) {
        if (role == Qt::DisplayRole)
            rv = devices.keys().at(index.row());
        else if (role == SimpleServiceBrowserModel::AddressRole)
            rv = devices.value(devices.keys().at(index.row()));
        else if (role == SimpleServiceBrowserModel::PortRole)
            rv = 1234;
    }

    return rv;
}
