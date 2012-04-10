/****************************************************************************

This file is part of the QtMediaHub project on http://www.qtmediahub.com

Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Girish Ramakrishnan girish@forwardbias.in
Contact:  Donald Carr sirspudd@gmail.com
Contact:  Nokia Corporation johannes.zellner@nokia.com

You may use this file under the terms of the BSD license as follows:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include "snesparser.h"
#include "libraryinfo.h"
#include "scopedtransaction.h"
#include "globalsettings.h"

#include <QtGui>
#include <QtSql>

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

SnesParser::SnesParser()
{
    QStringList additionalExtensions = m_settings->value(GlobalSettings::ExtraSNESExtensions).toString().split(",", QString::SkipEmptyParts);

    m_supportedTypes << "smc";

    if (!additionalExtensions.isEmpty()) {
        m_supportedTypes << additionalExtensions;
    }
}

bool SnesParser::canRead(const QFileInfo &info) const
{
    return m_supportedTypes.contains(info.suffix());
}

QString determineTitle(const QFileInfo &fi) {
    return fi.absoluteFilePath();
}

QByteArray determineThumbnail(const QFileInfo &fi) {
    return QByteArray();
}

// ## See if DELETE+INSERT is the best approach. Sqlite3 supports INSERT OR IGNORE which could aslo be used
// ## Also check other upsert methods
QList<QSqlRecord> SnesParser::updateMediaInfos(const QList<QFileInfo> &fis, const QString &searchPath, QSqlDatabase db)
{
    Q_UNUSED(searchPath);
    QList<QSqlRecord> records;
    QSqlQuery query(db);
    ScopedTransaction transaction(db);

    foreach(const QFileInfo &fi, fis) {
        DEBUG << "Updating " << fi.absoluteFilePath();
        query.prepare("DELETE FROM snes WHERE filepath=:filepath");
        query.bindValue(":filepath", fi.absoluteFilePath());
        if (!query.exec())
            qWarning() << query.lastError().text();

        if (!query.prepare("INSERT INTO snes (filepath, title, thumbnail, uri, directory, mtime, ctime, filesize) "
                           " VALUES (:filepath, :title, :thumbnail, :uri, :directory, :mtime, :ctime, :filesize)")) {
            qWarning() << query.lastError().text();
            return records;
        }

        query.bindValue(":filepath", fi.absoluteFilePath());
        query.bindValue(":title", ::determineTitle(fi));
        query.bindValue(":thumbnail", ::determineThumbnail(fi));
        query.bindValue(":uri", QUrl::fromLocalFile(fi.absoluteFilePath()).toEncoded());

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
    }

    emit databaseUpdated(records);
    return records;
}
