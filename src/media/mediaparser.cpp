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

#include "mediaparser.h"
#include "scopedtransaction.h"
#include <QtSql>

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

MediaParser::MediaParser()
{
}

QHash<QString, MediaScanner::FileInfo> MediaParser::findFilesByPath(const QString &path, QSqlDatabase db)
{
    QHash<QString, MediaScanner::FileInfo> hash;
    QSqlQuery query(db);
    query.setForwardOnly(true);
    query.prepare(QString("SELECT filepath, mtime, ctime, filesize FROM %1 WHERE directory=:path").arg(type()));
    query.bindValue(":path", path);
    if (!query.exec()) {
        DEBUG << query.lastError().text();
        return hash;
    }

    while (query.next()) {
        MediaScanner::FileInfo fi;
        fi.name = query.value(0).toString();
        fi.mtime = query.value(1).toLongLong();
        fi.ctime = query.value(2).toLongLong();
        fi.size = query.value(3).toLongLong();

        hash.insert(fi.name, fi);
    }

    return hash;
}

