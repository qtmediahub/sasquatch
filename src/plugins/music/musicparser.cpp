#include "musicparser.h"
#include "tagreader.h"
#include "scopedtransaction.h"
#include <QtSql>

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

bool MusicParser::canRead(const QFileInfo &info) const
{
    return info.suffix() == "mp3";
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

QByteArray determineThumbnail(const TagReader &reader, const QFileInfo &fi)
{
    // Thumbnail is determined from following
    // 1. Embedded thumbnail
    // 2. foo.mp3 -> foo.{jpg,png,gif,bmp}
    // 3. {id3_album, cover, album, folder}.{jpg, png, gif, bmp}
    // 4. default image (empty)

    // 1
    QByteArray ba = reader.thumbnail();
    if (!ba.isNull())
        return ba;

    QDir dir = fi.absoluteDir();
    const char *supportedExtensions[] = { ".jpg", ".png", ".gif", ".bmp" }; // prioritized

    // 2
    for (unsigned i = 0; i < sizeof(supportedExtensions)/sizeof(char *); i++) {
        if (dir.exists(fi.baseName() + supportedExtensions[i]))
            return QByteArray("file://") + QFile::encodeName(dir.absoluteFilePath(fi.baseName())) + supportedExtensions[i];
    }

    // 3
    QString album = reader.album().simplified();
    for (unsigned i = 0; i < sizeof(supportedExtensions)/sizeof(char *); i++) {
        if (dir.exists(album + supportedExtensions[i]))
            return QByteArray("file://") + QFile::encodeName(dir.absoluteFilePath(album)) + supportedExtensions[i];
        if (dir.exists(QString("album") + supportedExtensions[i]))
            return QByteArray("file://") + QFile::encodeName(dir.absoluteFilePath("album")) + supportedExtensions[i];
        if (dir.exists(QString("cover") + supportedExtensions[i]))
            return QByteArray("file://") + QFile::encodeName(dir.absoluteFilePath("cover")) + supportedExtensions[i];
        if (dir.exists(QString("folder") + supportedExtensions[i]))
            return QByteArray("file://") + QFile::encodeName(dir.absoluteFilePath("folder")) + supportedExtensions[i];
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
QList<QSqlRecord> MusicParser::updateMediaInfos(const QList<QFileInfo> &fis)
{
    QList<QSqlRecord> records;
    QSqlQuery query(m_db);
    ScopedTransaction transaction(m_db);

    foreach(const QFileInfo &fi, fis) {
        DEBUG << "Updating " << fi.absoluteFilePath();
        TagReader reader(fi.absoluteFilePath());
        query.prepare("DELETE FROM music WHERE filepath=:filepath");
        query.bindValue(":filepath", fi.absoluteFilePath());
        if (!query.exec())
            DEBUG << query.lastError().text();

        if (!query.prepare("INSERT INTO music (filepath, title, album, artist, track, year, genre, comment, thumbnail, length, bitrate, samplerate, directory, mtime, ctime, filesize) "
                           " VALUES (:filepath, :title, :album, :artist, :track, :year, :genre, :comment, :thumbnail, :length, :bitrate, :samplerate, :directory, :mtime, :ctime, :filesize)")) {
            DEBUG << query.lastError().text();
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
        query.bindValue(":thumbnail", determineThumbnail(reader, fi));

        query.bindValue(":length", reader.length());
        query.bindValue(":bitrate", reader.bitrate());
        query.bindValue(":samplerate", reader.sampleRate());

        query.bindValue(":directory", fi.absolutePath());
        query.bindValue(":mtime", fi.lastModified().toTime_t());
        query.bindValue(":ctime", fi.created().toTime_t());
        query.bindValue(":filesize", fi.size());

        if (!query.exec())
            DEBUG << query.lastError().text();
        
        QSqlRecord record;
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


