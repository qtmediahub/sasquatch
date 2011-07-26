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
#include <QHash>
#include "global.h"

class MediaParser;

// MediaScanner is designed to be run in a separate thread. Do not call the methods
// below directly from the ui thread. None of the methods below are thread-safe!
// except stop()
class QMH_EXPORT MediaScanner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentScanPath READ currentScanPath NOTIFY currentScanPathChanged)

public:
    MediaScanner(QObject *parent = 0);
    ~MediaScanner();

    struct FileInfo {
        QString name;
        quint32 mtime;
        quint32 ctime;
        qint64 size;

        bool valid() const { return !name.isEmpty(); }
    };

    void stop() { m_stop = true; }

    QString currentScanPath() const { return m_currentScanPath; }

public slots:
    void initialize();
    void addSearchPath(const QString &type, const QString &path, const QString &name);
    void refresh(const QString &type = QString());
    void addParser(MediaParser *);
    
signals:
    void currentScanPathChanged();
    void scanFinished();

private:
    void scan(MediaParser *parser, const QString &path);

    static MediaScanner *s_instance;
    volatile bool m_stop;
    QSqlDatabase m_db;
    QString m_errorString;

    QString m_currentScanPath;

    QHash<QString, MediaParser *> m_parsers;
};

#endif // MEDIASCANNER_H

