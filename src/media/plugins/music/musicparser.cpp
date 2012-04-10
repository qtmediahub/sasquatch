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

#include "musicparser.h"
#include "tagreader.h"
#include "scopedtransaction.h"
#include "libraryinfo.h"
#include "lastfmprovider.h"

#include <QtSql>
#include <QtNetwork>

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

bool MusicParser::canRead(const QFileInfo &info) const
{
    static QStringList extensions
            = QStringList() << "mp3"
                            << "aac"
                            << "mp4"
                            << "m4a"
                            << "ogg"
                            << "flac";

    return extensions.contains(info.suffix());
}

QString cleanString(QString str)
{
    str = str.simplified();
    if (!str.isEmpty())
        str[0] = str[0].toUpper();
    return str;
}

QString determineTitle(const TagReader &reader, const QFileInfo &fi)
{
    QString title = reader.title();
    title = title.simplified();

    // Many mp3 state the title as 'Track xx' which is as good as empty
    if (title.startsWith("Track ", Qt::CaseInsensitive))
        title.clear();

    if (title.isEmpty())
        title = fi.baseName();

    title[0] = title[0].toUpper();

    return title;
}

QString determineAlbum(const TagReader &reader, const QFileInfo &fi)
{
    QString album = reader.album();
    album = album.simplified();

    if (album.isEmpty())
        album = fi.dir().dirName();

    album[0] = album[0].toUpper();

    return album;
}

QByteArray determineThumbnail(GlobalSettings *settings, const TagReader &reader, const QFileInfo &fi)
{
    QByteArray md5 = QCryptographicHash::hash("file://" + QFile::encodeName(fi.absoluteFilePath()), QCryptographicHash::Md5).toHex();
    QFileInfo thumbnailInfo(LibraryInfo::thumbnailPath(settings) + md5 + ".png");
    if (thumbnailInfo.exists())
        return QUrl::fromLocalFile(thumbnailInfo.absoluteFilePath()).toEncoded();

    // Thumbnail is determined from following
    // 1. Embedded thumbnail
    // 2. foo.mp3 -> foo.{jpg,png,gif,bmp}
    // 3. {id3_album, cover, album, folder}.{jpg, png, gif, bmp}
    // 4. default image (empty)

    // 1
    QImage img = reader.thumbnailImage();
    if (!img.isNull()) {
        img.save(thumbnailInfo.absoluteFilePath());
        return QUrl::fromLocalFile(thumbnailInfo.absoluteFilePath()).toEncoded();
    }

    QDir dir = fi.absoluteDir();
    const char *supportedExtensions[] = { ".jpg", ".png", ".gif", ".bmp" }; // prioritized

    // 2
    for (unsigned i = 0; i < sizeof(supportedExtensions)/sizeof(char *); i++) {
        if (dir.exists(fi.baseName() + supportedExtensions[i]))
            return QUrl::fromLocalFile(dir.absoluteFilePath(fi.baseName()) + supportedExtensions[i]).toEncoded();
    }

    // 3
    QString album = reader.album().simplified();
    for (unsigned i = 0; i < sizeof(supportedExtensions)/sizeof(char *); i++) {
        if (dir.exists(album + supportedExtensions[i]))
            return QUrl::fromLocalFile(dir.absoluteFilePath(album) + supportedExtensions[i]).toEncoded();
        if (dir.exists(QString("album") + supportedExtensions[i]))
            return QUrl::fromLocalFile(dir.absoluteFilePath("album") + supportedExtensions[i]).toEncoded();
        if (dir.exists(QString("cover") + supportedExtensions[i]))
            return QUrl::fromLocalFile(dir.absoluteFilePath("cover") + supportedExtensions[i]).toEncoded();
        if (dir.exists(QString("folder") + supportedExtensions[i]))
            return QUrl::fromLocalFile(dir.absoluteFilePath("folder") + supportedExtensions[i]).toEncoded();
    }

    return QByteArray();
}

QString determineArtist(const TagReader &reader)
{
    QString artist = cleanString(reader.artist());
    if (artist.isEmpty())
        return "Unknown Artist";
    return artist;
}

// ## See if DELETE+INSERT is the best approach. Sqlite3 supports INSERT OR IGNORE which could aslo be used
// ## Also check other upsert methods
QList<QSqlRecord> MusicParser::updateMediaInfos(const QList<QFileInfo> &fis, const QString &searchPath, QSqlDatabase db)
{
    Q_UNUSED(searchPath);
    QList<QSqlRecord> records;
    QSqlQuery query(db);
    ScopedTransaction transaction(db);

    foreach(const QFileInfo &fi, fis) {
        DEBUG << "Updating " << fi.absoluteFilePath();
        TagReader reader(fi.absoluteFilePath());
        query.prepare("DELETE FROM music WHERE filepath=:filepath");
        query.bindValue(":filepath", fi.absoluteFilePath());
        if (!query.exec())
            qWarning() << query.lastError().text();

        if (!query.prepare("INSERT INTO music (filepath, title, album, artist, track, year, genre, comment, thumbnail, uri, length, bitrate, samplerate, directory, mtime, ctime, filesize) "
                           " VALUES (:filepath, :title, :album, :artist, :track, :year, :genre, :comment, :thumbnail, :uri, :length, :bitrate, :samplerate, :directory, :mtime, :ctime, :filesize)")) {
            qWarning() << query.lastError().text();
            return records;
        }

        query.bindValue(":filepath", fi.absoluteFilePath());
        query.bindValue(":title", determineTitle(reader, fi));
        query.bindValue(":album", determineAlbum(reader, fi));
        query.bindValue(":artist", determineArtist(reader));
        query.bindValue(":track", reader.track());
        query.bindValue(":year", reader.year());
        query.bindValue(":genre", reader.genre());
        query.bindValue(":comment", reader.comment());
        query.bindValue(":thumbnail", determineThumbnail(m_settings, reader, fi));
        query.bindValue(":uri", QUrl::fromLocalFile(fi.absoluteFilePath()).toEncoded());

        query.bindValue(":length", reader.length());
        query.bindValue(":bitrate", reader.bitrate());
        query.bindValue(":samplerate", reader.sampleRate());

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

void MusicParser::runExtraMetaDataProvider(QSqlDatabase db)
{
    DEBUG << "run extra meta data provider for music";

    LastFMProvider *lastFmProvider = new LastFMProvider(db);
    connect(lastFmProvider, SIGNAL(allRequestsFinished()), lastFmProvider, SLOT(deleteLater()));
}


