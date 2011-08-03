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

#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QList>
#include <QDateTime>
#include <QtSql>
#include <QDesktopServices>

#include "global.h"

#include "media/mediaplugin.h"

class QUrl;
class BackendPrivate;
class Skin;
class Frontend;
class QAction;
class MediaScanner;
class QDeclarativeView;
class Skin;

class QMH_EXPORT Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<Skin*> skins READ skins NOTIFY skinsChanged)
    Q_PROPERTY(QString pluginPath READ pluginPath NOTIFY pluginPathChanged)
    Q_PROPERTY(QString resourcePath READ resourcePath NOTIFY resourcePathChanged)
    Q_PROPERTY(QObject *targetsModel READ targetsModel NOTIFY targetsModelChanged)

public:
    static Backend *instance();
    ~Backend();

    static QString storageLocation(QDesktopServices::StandardLocation type);

    QString language() const;

    QList<Skin*> skins() const;

    QString basePath() const;
    QString pluginPath() const;
    QString resourcePath() const;
    QString thumbnailPath() const;

    Q_INVOKABLE void openUrlExternally(const QUrl &url) const;
    Q_INVOKABLE void log(const QString &logMsg);

    QObject *targetsModel() const;

    Frontend *frontend() const;

    MediaScanner *mediaScanner() const;
    QSqlDatabase mediaDatabase() const;

    Q_INVOKABLE void loadEngines();
    Q_INVOKABLE QStringList findApplications() const;

    void setPrimarySession(bool);
    void registerDeclarativeFrontend(QDeclarativeView *view, Skin* skin);

public slots:
    void initialize();

signals:
    void skinsChanged();
    void pluginPathChanged();
    void resourcePathChanged();
    void backendChanged();
    void targetsModelChanged();

private:
    explicit Backend(QObject *parent = 0);
    static Backend *s_instance;
    BackendPrivate *d;
};

#endif // BACKEND_H
