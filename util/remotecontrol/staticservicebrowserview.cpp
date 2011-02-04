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

StaticServiceBrowserView::StaticServiceBrowserView(QWidget *parent)
    : QTreeView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);

    m_model = new QStandardItemModel(this);
    QString servicesFile = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/services.conf";
    if (QFile::exists(servicesFile)) {
        initModelFromFile(servicesFile);
    } else {
        initModelFromFile(":/services.conf");
    }
    setModel(m_model);
    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(handleActivated(QModelIndex)));

    for (int i = 0; i < m_model->columnCount(); i++)
        resizeColumnToContents(i);

    selectionModel()->select(m_model->index(0, 0), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);

    save();
}

StaticServiceBrowserView::~StaticServiceBrowserView()
{
}

void StaticServiceBrowserView::initModelFromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("Failed to open static services file");
        return;
    }

    QStringList headerLabels;
    headerLabels << StaticServiceBrowserView::tr("Host Name")
                 << StaticServiceBrowserView::tr("IP")
                 << StaticServiceBrowserView::tr("Port");
    m_model->setHorizontalHeaderLabels(headerLabels);

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

void StaticServiceBrowserView::handleActivated(const QModelIndex &index)
{
    QStandardItem *item = m_model->invisibleRootItem();
    QString hostName = item->child(index.row(), 0)->text();
    Q_UNUSED(hostName);
    QString ip = item->child(index.row(), 1)->text();
    QString port = item->child(index.row(), 2)->text();

    emit serviceSelected(QHostAddress(ip), port.toInt());
}

void StaticServiceBrowserView::addService(const QString &hostName, const QString &ip, const QString &port)
{
    QList<QStandardItem *> columns;
    columns.append(new QStandardItem);
    columns[0]->setText(hostName);
    columns[0]->setEditable(false);
    columns.append(new QStandardItem);
    columns[1]->setText(ip);
    columns[1]->setEditable(false);
    columns.append(new QStandardItem);
    columns[2]->setText(port);
    columns[2]->setEditable(false);

    m_model->appendRow(columns);

    if (model()) { // ## ugly: used to distinguish if called by user or initModelFromFile
        save();
    }
}

void StaticServiceBrowserView::removeService()
{
    m_model->removeRow(currentIndex().row());
    save();
}

void StaticServiceBrowserView::save()
{
    QString servicesFileName = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/services.conf";
    QFile file(servicesFileName);
    file.open(QFile::WriteOnly);
    QTextStream stream(&file);
    stream << "# Static configuration file auto-created by qmh remotecontrol" << endl;
    QStandardItem *root = m_model->invisibleRootItem();
    for (int i = 0; i < root->rowCount(); i++) {
        QString hostName = root->child(i, 0)->text();
        QString ip = root->child(i, 1)->text();
        QString port = root->child(i, 2)->text();
        stream << hostName << " " << ip << " " << port << endl;
    }
}
