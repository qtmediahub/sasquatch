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

#include "backend.h"
#include "frontend.h"

#include "qmh-config.h"
#include "rpc/rpcconnection.h"
#include "skin.h"
#include "scopedtransaction.h"
#include "media/mediaplugin.h"
#include "media/mediascanner.h"
#include "media/mediamodel.h"
#include "media/mediaparser.h"

#include "httpserver/httpserver.h"

#ifdef QT_SINGLE_APPLICATION
#include "qtsingleapplication.h"
#endif

#include <QDir>
#include <QString>
#include <QPluginLoader>
#include <QCoreApplication>
#include <QVariant>
#include <QFileSystemModel>
#include <QDesktopServices>
#include <QFileSystemWatcher>
#include <QNetworkProxy>
#include <QDeclarativeView>
#include <QHostInfo>

#ifdef QMH_AVAHI
#include "qavahiservicepublisher.h"
#endif

#include "libraryinfo.h"

#include <QDebug>

class BackendPrivate : public QObject
{
    Q_OBJECT
public:
    BackendPrivate(Backend *p);
    ~BackendPrivate();

public:
    void ensureStandardPaths();

    bool primarySession;

    bool remoteControl;

    HttpServer *httpServer;

    Backend *q;
};

BackendPrivate::BackendPrivate(Backend *p)
    : QObject(p),
      primarySession(true),
      remoteControl(Config::isEnabled("remote", false)),
      httpServer(0),
      q(p)
{
    ensureStandardPaths();

    httpServer = new HttpServer(Config::value("stream-port", "1337").toInt(), this);

    MediaScanner::instance();
}

BackendPrivate::~BackendPrivate()
{
}

Backend::Backend(QObject *parent)
    : QObject(parent),
      d(new BackendPrivate(this))
{
#ifdef QMH_AVAHI
    if (d->primarySession
            && !d->remoteControl
            && Config::isEnabled("avahi", true)
            && Config::isEnabled("avahi-advertize", true)) {
        QAvahiServicePublisher *publisher = new QAvahiServicePublisher(this);
        publisher->publish(QHostInfo::localHostName(), "_qtmediahub._tcp", 1234, "Qt Media Hub JSON-RPCv2 interface");
        qDebug() << "Advertizing session via zeroconf";
    } else {
        qDebug() << "Failing to advertize session via zeroconf";
    }
#endif
}

Backend::~Backend()
{
    MediaScanner::destroy();

    delete d;
    d = 0;
}

void BackendPrivate::ensureStandardPaths()
{
    QDir dir;
    dir.mkpath(LibraryInfo::thumbnailPath());
    dir.mkpath(LibraryInfo::dataPath());
}

QString Backend::resourcePath() const
{
    return LibraryInfo::resourcePath();
}

void Backend::openUrlExternally(const QUrl & url) const
{
    QDesktopServices::openUrl(url);
}

QStringList Backend::findApplications() const
{
    QStringList apps;
    foreach(const QString &appSearchPath, LibraryInfo::applicationPaths()) {
        QStringList subdirs = QDir(appSearchPath).entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
        foreach(const QString &subdir, subdirs)  {
            QString appPath(appSearchPath + '/' + subdir + '/');
            QString fileName(appPath + "qmhmanifest.qml"); // look for apps/x/qmhmanifest.qml
            QFile prospectiveFile(fileName);
            if (prospectiveFile.exists())
                apps << (QDir(appPath).absolutePath() + '/');
        }
    }
    return apps;
}

void Backend::setPrimarySession(bool primarySession)
{
    d->primarySession = primarySession;
}

void Backend::registerQmlProperties(QDeclarativePropertyMap *runtime)
{
    if (!d->remoteControl) {
        runtime->insert("mediaScanner", qVariantFromValue(static_cast<QObject *>(MediaScanner::instance())));
        runtime->insert("httpServer", qVariantFromValue(static_cast<QObject *>(d->httpServer)));
    }
    runtime->insert("backend", qVariantFromValue(static_cast<QObject *>(this)));
}

#include "backend.moc"
