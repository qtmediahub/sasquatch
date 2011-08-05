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

#ifndef MEDIASCANNER_H
#define MEDIASCANNER_H

#include <QObject>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QStringList>
#include <QHash>
#include "global.h"

class MediaParser;
class MediaScannerWorker;
class QThread;

class QMH_EXPORT MediaScanner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentScanPath READ currentScanPath NOTIFY currentScanPathChanged)

public:
    static MediaScanner *instance();
    static void destroy();
    ~MediaScanner();

    struct FileInfo {
        int rowid;
        QString name;
        quint32 mtime;
        quint32 ctime;
        qint64 size;

        bool valid() const { return !name.isEmpty(); }
    };

    Q_INVOKABLE void stop();

    QString currentScanPath() const { return m_currentScanPath; }

    Q_INVOKABLE void addSearchPath(const QString &type, const QString &path, const QString &name);
    Q_INVOKABLE void removeSearchPath(const QString &type, const QString &path);
    Q_INVOKABLE QStringList searchPaths(const QString &type) const;

    Q_INVOKABLE void refresh(const QString &type = QString());
    Q_INVOKABLE void addParser(MediaParser *);
    
signals:
    void currentScanPathChanged();
    void scanStarted(const QString &type);
    void scanFinished(const QString &type);
    void searchPathAdded(const QString &type, const QString &path, const QString &name);
    void searchPathRemoved(const QString &type, const QString &path);

private slots:
    void handleScanPathChanged(const QString &scanPath);

private:
    explicit MediaScanner(QObject *parent = 0);
    void ensureDatabase();
    void loadParserPlugins();

    QThread *m_workerThread;
    MediaScannerWorker *m_worker;
    QString m_currentScanPath;
    
    static MediaScanner *s_instance;
    friend class MediaScannerWorker;
};

#endif // MEDIASCANNER_H

