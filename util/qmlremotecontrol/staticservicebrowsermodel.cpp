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

#include "staticservicebrowserview.h"
#include <QtGui>

StaticServiceBrowserModel::StaticServiceBrowserModel(QWidget *parent)
    : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();
    roleNames[Qt::DisplayRole] = "display";
    roleNames[AddressRole] = "address";
    roleNames[PortRole] = "port";
    setRoleNames(roleNames);

    QString servicesFile = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/services.conf";
    if (QFile::exists(servicesFile)) {
        initModelFromFile(servicesFile);
    } else {
        initModelFromFile(":/services.conf");
    }
    save();
}

StaticServiceBrowserModel::~StaticServiceBrowserModel()
{
}

void StaticServiceBrowserModel::initModelFromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("Failed to open static services file");
        return;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString hostName;
        stream >> hostName;
        if (hostName.startsWith('#') || hostName.isEmpty()) {
            stream.readLine();
            continue;
        }
        QString ip, port;
        stream >> ip >> port;

        addService(hostName, ip, port);
    }
}

void StaticServiceBrowserModel::addService(const QString &hostName, const QString &ip, const QString &port)
{
    QStringList list;
    list << hostName << ip << port;

    int start = m_model.count()-1 < 0 ? 0 : m_model.count()-1;
    int end = m_model.count() < 0 ? 1 : m_model.count();

    emit beginInsertRows(QModelIndex(), start, end);
    m_model.append(list);
    emit endInsertRows();

    save();
}

void StaticServiceBrowserModel::save()
{
    QString servicesFileName = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/services.conf";
    QFile file(servicesFileName);
    file.open(QFile::WriteOnly);
    QTextStream stream(&file);
    stream << "# Static configuration file auto-created by qmh remotecontrol" << endl;
    for (int i = 0; i < m_model.count(); i++) {
        QString hostName = m_model.at(i).at(0);
        QString ip = m_model.at(i).at(1);
        QString port = m_model.at(i).at(2);
        stream << hostName << " " << ip << " " << port << endl;
    }
}

QVariant StaticServiceBrowserModel::data(const QModelIndex &index, int role) const
{
    QVariant rv;

    if (!index.isValid())
        return rv;

    if (0 <= index.row() && index.row() < m_model.count()) {
        if (role == Qt::DisplayRole)
            return m_model.at(index.row()).at(0);
        else if (role == StaticServiceBrowserModel::AddressRole)
            return m_model.at(index.row()).at(1);
        else if (role == StaticServiceBrowserModel::PortRole)
            return m_model.at(index.row()).at(2);
    }

    return rv;
}
