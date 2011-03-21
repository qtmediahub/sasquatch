/****************************************************************************
 *
 * This file is part of the QtMediaHub project on http://www.gitorious.org.
 *
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).*
 * All rights reserved.
 *
 * Contact:  Nokia Corporation (qt-info@nokia.com)**
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 *
 * ****************************************************************************/

#include "appstore.h"

#include <QDeclarativeComponent>

AppStore::AppStore(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
{
    installer = new AppInstallerInterface("com.nokia.appstore.installer", "/", QDBusConnection::sessionBus(), this);

    connect(installer, SIGNAL(installAppFinished(quint32,QString)), this, SIGNAL(installAppFinished(quint32,QString)));
    connect(installer, SIGNAL(installAppFailed(quint32,QString,QString)), this, SIGNAL(installAppFailed(quint32,QString,QString)));
    connect(installer, SIGNAL(installAppProgress(quint32,QString,int)), this, SIGNAL(installAppProgress(quint32,QString,int)));

    connect(installer, SIGNAL(installAppFinished(quint32,QString)), this, SLOT(refresh()));

    refresh();
}

void AppStore::installApp(const QString &name, const QString &appUuidStr, const QString &uri)
{
    installer->installApp(getuid(), appUuidStr, uri, name, 0, QStringList());
}

void AppStore::deleteApp(const QString &name, const QString &appUuidStr, bool keepDocuments)
{
    installer->deleteApp(getuid(), appUuidStr, keepDocuments);
}

void AppStore::refresh()
{
    QDir appsDir("/home/jzellner/projects/qtmediahub/hub/resources/apps/");
    appsDir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::NoSymLinks);

    // empty list
    while (!mApps.isEmpty())
         delete mApps.takeFirst();

    QDirIterator it(appsDir);
    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo= it.fileInfo();

        AppInfo *info = readApplicationFolder(fileInfo);
        if (info)
            mApps << info;
    }

    emit appsChanged();
}

AppInfo * AppStore::readApplicationFolder(const QFileInfo &fileInfo)
{
    QFileInfo desktopFile = fileInfo.absoluteFilePath() + "/app.desktop";
    if (!desktopFile.exists()) {
        qDebug("[AppStore] [%s] app.desktop does not exist", fileInfo.absoluteFilePath().toLocal8Bit().constData());
        return 0;
    }

    QSettings content(desktopFile.absoluteFilePath(), QSettings::IniFormat);

    // sanity checks
    if (!content.childGroups().contains("Desktop Entry")) {
        qDebug("[AppStore] [%s] app.desktop does not seem to be a desktop-file", fileInfo.absoluteFilePath().toLocal8Bit().constData());
        return 0;
    }

    content.beginGroup("Desktop Entry");

    if (!content.contains("Name") || !content.contains("Icon") || !content.contains("Exec")) {
        qDebug("[AppStore] [%s] app.desktop does not contain needed values", fileInfo.absoluteFilePath().toLocal8Bit().constData());
        return 0;
    }

    AppInfo *appInfo = new AppInfo(fileInfo.absoluteFilePath()
                                   , content.value("Name", "").toString()
                                   , content.value("Icon", "").toString()
                                   , content.value("Comment", "").toString()
                                   , content.value("Version", "").toString()
                                   , content.value("Exec", "").toString()
                                   , content.value("Uuid", "").toString()
                                   , content.value("Categories", "").toString()
                                   );
    return appInfo;
}
