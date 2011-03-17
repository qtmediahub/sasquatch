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

#ifndef APPINSTALLERINTERFACE_H
#define APPINSTALLERINTERFACE_H

#include <QtDBus>

class AppInstallerInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName() { return "com.nokia.appstore.installer"; }
    AppInstallerInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);
    ~AppInstallerInterface();

signals:
    void installAppProgress(quint32 userId, const QString &appUuid, int progress);
    void installAppFinished(quint32 userId, const QString &appUuid);
    void installAppFailed(quint32 userId, const QString &appUuid, const QString &error);

public slots:
    inline QDBusPendingReply<> createUser(const QString &name)
    {
        QList<QVariant> argumentList;
        argumentList << name;

        return asyncCallWithArgumentList(QLatin1String("createUser"), argumentList);
    }

    inline QDBusPendingReply<> deleteUser(quint32 userId, bool keepDocuments)
    {
        QList<QVariant> argumentList;
        argumentList << userId;
        argumentList << keepDocuments;

        return asyncCallWithArgumentList(QLatin1String("deleteUser"), argumentList);
    }

    inline QDBusPendingReply<> installApp(quint32 userId, const QString &appUuidStr, const QString &downloadUrl, const QString &name, quint64 installedSize, const QStringList &capabilities)
    {
        QList<QVariant> argumentList;
        argumentList << userId;
        argumentList << appUuidStr;
        argumentList << downloadUrl;
        argumentList << name;
        argumentList << installedSize;
        argumentList << capabilities;

        return asyncCallWithArgumentList(QLatin1String("installApp"), argumentList);
    }

    inline QDBusPendingReply<> deleteApp(quint32 userId, const QString &appUuidStr, bool keepDocuments)
    {
        QList<QVariant> argumentList;
        argumentList << userId;
        argumentList << appUuidStr;
        argumentList << keepDocuments;

        return asyncCallWithArgumentList(QLatin1String("deleteApp"), argumentList);
    }
};

#endif // APPINSTALLERINTERFACE_H
