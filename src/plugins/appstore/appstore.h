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

#ifndef APPSTORE_H
#define APPSTORE_H

#include <QObject>
#include <QDir>
#include <QDebug>
#include <QDeclarativeItem>

#include "appinstallerinterface.h"
#include "appinfo.h"

class AppStore : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath)
    Q_PROPERTY(QList<QObject*> apps READ apps NOTIFY appsChanged)

public:
    AppStore(QDeclarativeItem *parent = 0);

    Q_INVOKABLE void installApp(const QString &name, const QString &appUuidStr, const QString &uri);
    Q_INVOKABLE void deleteApp(const QString &name, const QString &appUuidStr, bool keepDocuments);

    QString path() const { return mPath; }
    QList<QObject*> apps() const { return mApps; }

public slots:
    void setPath(const QString &path) { mPath = path;/* searchPath();*/ }
    void refresh();

signals:
    void installAppProgress(quint32 userId, const QString &appUuid, int progress);
    void installAppFinished(quint32 userId, const QString &appUuid);
    void installAppFailed(quint32 userId, const QString &appUuid, const QString &error);

    void appsChanged();

private:
    AppInfo *readApplicationFolder(const QFileInfo &fileInfo);

    AppInstallerInterface *installer;
    QString mPath;
    QList<QObject*> mApps;
};

#endif // APPSTORE_H
