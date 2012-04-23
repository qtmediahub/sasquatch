/** This file is part of Qt Media Hub**

Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation qmh-development@qt-project.org

You may use this file under the terms of the BSD license
as follows:

Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of Nokia Corporation and its Subsidiary(-ies)
nor the names of its contributors may be used to endorse or promote
products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. **/

#include "radioparser.h"

#include "scopedtransaction.h"
#include <QtSql>

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

bool RadioParser::canRead(const QFileInfo &info) const
{
    static QStringList extensions
            = QStringList() << "pls"
                            << "asx";

    return extensions.contains(info.suffix());
}

QString cleanString(QString str)
{
    str = str.simplified();
    if (!str.isEmpty())
        str[0] = str[0].toUpper();
    return str;
}

struct RadioInfo {
    QUrl uri;
    QString title;
    QString thumbnail;
    int length;
};

static RadioInfo *readPLS(QFileInfo fileInfo)
{
    QSettings settings(fileInfo.filePath(), QSettings::IniFormat);
    QStringList keys = settings.allKeys();

    if (!keys.contains("playlist/NumberOfEntries", Qt::CaseInsensitive))
        return 0;

    if (settings.value("playlist/NumberOfEntries", 0).toInt() <= 0)
        return 0;

    // currently only choose first title
    if (    !keys.contains("playlist/File1", Qt::CaseInsensitive) ||
            !keys.contains("playlist/Title1", Qt::CaseInsensitive) ||
            !keys.contains("playlist/Length1", Qt::CaseInsensitive) )
        return 0;

    QString file = settings.value("playlist/File1", "").toString();
    QString title = settings.value("playlist/Title1", "").toString();
    int length = settings.value("playlist/Length1", "").toInt();

    RadioInfo *info = new RadioInfo();
    info->uri = QUrl(file);
    info->title = title;
    info->length = length;

    QRegExp e("\\(#[0-9]+ - [0-9]+/[0-9]+\\) ");
    int regExpIndex = e.indexIn(title);
    int regExpLenght = e.matchedLength();

    if(regExpIndex == 0)
        info->title = title.right(title.length() - regExpLenght);

    return info;
}

static RadioInfo *readASX(QFileInfo fileInfo)
{
    QFile file(fileInfo.filePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return 0;

    QString title;
    QUrl uri;
    QString banner;

    QXmlStreamReader reader(&file);
    while(!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        if (token == QXmlStreamReader::StartDocument)
            continue;

        if (token == QXmlStreamReader::StartElement) {
            if (reader.name() == "entry") {
                continue;
            }

            if (reader.name() == "Banner" && reader.attributes().hasAttribute("href")) {
                banner = reader.attributes().value("href").toString();
                continue;
            }

            if (reader.name() == "title") {
                reader.readNext();
                if (reader.tokenType() == QXmlStreamReader::Characters)
                    title = reader.text().toString();
                continue;
            }
            if (reader.name() == "ref" && reader.attributes().hasAttribute("href")) {
                uri = reader.attributes().value("href").toString();
                continue;
            }
        }

        if (title != "" && uri.isEmpty())
            break;
    }

    if (reader.hasError() || title == "" || uri.isEmpty()) {
        reader.clear();
        return 0;
    }

    reader.clear();

    RadioInfo *info = new RadioInfo();
    info->uri = QUrl(uri);
    info->title = title;
    info->thumbnail = banner;

    return info;
}

// ## See if DELETE+INSERT is the best approach. Sqlite3 supports INSERT OR IGNORE which could aslo be used
// ## Also check other upsert methods
QList<QSqlRecord> RadioParser::updateMediaInfos(const QList<QFileInfo> &fis, const QString &searchPath, QSqlDatabase db)
{
    Q_UNUSED(searchPath);
    QList<QSqlRecord> records;
    QSqlQuery query(db);
    ScopedTransaction transaction(db);

    foreach(const QFileInfo &fi, fis) {
        DEBUG << "Updating " << fi.absoluteFilePath();

        RadioInfo *info = 0;
        if (fi.suffix() == "pls")
            info = readPLS(fi);
        else if (fi.suffix() == "asx")
            info = readASX(fi);

        if (!info)
            continue;

        query.prepare("DELETE FROM radio WHERE filepath=:filepath");
        query.bindValue(":filepath", fi.absoluteFilePath());
        if (!query.exec())
            qWarning() << query.lastError().text();

        if (!query.prepare("INSERT INTO radio (filepath, title, thumbnail, length, uri, directory, mtime, ctime, filesize) "
                           " VALUES (:filepath, :title, :thumbnail, :length, :uri, :directory, :mtime, :ctime, :filesize)")) {
            qWarning() << query.lastError().text();
            if (info)
                delete info;
            return records;
        }

        query.bindValue(":filepath", fi.absoluteFilePath());
        query.bindValue(":title", info->title);
        query.bindValue(":thumbnail", info->thumbnail);

        query.bindValue(":length", info->length);
        query.bindValue(":uri", info->uri.toEncoded());

        query.bindValue(":directory", fi.absolutePath() + '/');
        query.bindValue(":mtime", fi.lastModified().toTime_t());
        query.bindValue(":ctime", fi.created().toTime_t());
        query.bindValue(":filesize", fi.size());

        if (!query.exec())
            qWarning() << query.lastError().text();
        
        QSqlRecord record;
        record.append(QSqlField("id", QVariant::Int));
        record.setValue(0, query.lastInsertId());
        QMap<QString, QVariant> boundValues = query.boundValues();
        for (QMap<QString, QVariant>::const_iterator it = boundValues.constBegin(); it != boundValues.constEnd(); ++it) {
            QString key = it.key().mid(1); // remove the ':'
            record.append(QSqlField(key, (QVariant::Type) it.value().type()));
            record.setValue(key, it.value());
        }
        records.append(record);

        if (info)
            delete info;
    }

    emit databaseUpdated(records);
    return records;
}


