#include "mediascanner.h"
#include <QtSql>
#include "tagreader.h"
#include "scopedtransaction.h"

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

const QString CONNECTION_NAME("MediaScanner");
const int BULK_LIMIT = 10;

MediaScanner::MediaScanner(const QSqlDatabase &db, QObject *parent)
    : QObject(parent), m_stop(false)
{
    m_db = QSqlDatabase::cloneDatabase(db, CONNECTION_NAME);
    if (!m_db.open())
        DEBUG << "Erorr opening database" << m_db.lastError().text();
    QSqlQuery query(db);
    //query.exec("PRAGMA synchronous=OFF"); // dangerous, can corrupt db
    //query.exec("PRAGMA journal_mode=WAL");
    query.exec("PRAGMA count_changes=OFF");
}

MediaScanner::~MediaScanner()
{
    QSqlDatabase::removeDatabase(CONNECTION_NAME);
}

void MediaScanner::addSearchPath(const QString &_path, const QString &name)
{
    Q_UNUSED(name); // ## Add this to the database
    QString path = QFileInfo(_path).absoluteFilePath();
    if (path.endsWith('/'))
        path.chop(1);

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO directories (path) VALUES (:path)");
    query.bindValue(":path", path);
    if (!query.exec()) {
        m_errorString = query.lastError().text();
        DEBUG << m_errorString;
        return;
    }

    scan(path);
}

QHash<QString, MediaScanner::FileInfo> MediaScanner::findFilesByPath(const QString &path)
{
    QHash<QString, MediaScanner::FileInfo> hash;
    QSqlQuery query(m_db);
    query.setForwardOnly(true);
    query.prepare("SELECT filepath, mtime, ctime, filesize FROM music WHERE directory=:path");
    query.bindValue(":path", path);
    if (!query.exec()) {
        m_errorString = query.lastError().text();
        DEBUG << m_errorString;
        return hash;
    }

    while (query.next()) {
        FileInfo fi;
        fi.name = query.value(0).toString();
        fi.mtime = query.value(1).toLongLong();
        fi.ctime = query.value(2).toLongLong();
        fi.size = query.value(3).toLongLong();

        hash.insert(fi.name, fi);
    }

    return hash;
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
    if (!ba.isNull()) {
        qDebug() << "found in exif";
        return ba;
    }

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
void MediaScanner::updateMediaInfos(const QList<QFileInfo> &fis)
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
            return;
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

        if (m_stop)
            break;
    }

    emit databaseUpdated(records);
}

static bool isMediaFile(const QFileInfo &info)
{
    return info.suffix() == "mp3";
}

void MediaScanner::scan(const QString &path)
{
    QQueue<QString> dirQ;
    dirQ.enqueue(path);

    QList<QFileInfo> diskFileInfos;

    while (!dirQ.isEmpty() && !m_stop) {
        QString curdir = dirQ.dequeue();
        QFileInfoList fileInfosInDisk = QDir(curdir).entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot|QDir::NoSymLinks);
        QHash<QString, FileInfo> fileInfosInDb = findFilesByPath(curdir);

        DEBUG << "Scanning " << curdir << fileInfosInDisk.count() << " files in disk and " << fileInfosInDb.count() << "in database";

        foreach(const QFileInfo &diskFileInfo, fileInfosInDisk) {
            FileInfo dbFileInfo = fileInfosInDb.take(diskFileInfo.absoluteFilePath());

            if (diskFileInfo.isFile()) {
                if (!isMediaFile(diskFileInfo))
                    continue;

                if (diskFileInfo.lastModified().toTime_t() == dbFileInfo.mtime
                    && diskFileInfo.created().toTime_t() == dbFileInfo.ctime
                    && diskFileInfo.size() == dbFileInfo.size) {
                    DEBUG << diskFileInfo.absoluteFilePath() << " : no change";
                    continue;
                }

                diskFileInfos.append(diskFileInfo);
                if (diskFileInfos.count() > BULK_LIMIT) {
                    updateMediaInfos(diskFileInfos);
                    diskFileInfos.clear();
                }
                DEBUG << diskFileInfo.absoluteFilePath() << " : added";
            } else if (diskFileInfo.isDir()) {
                dirQ.enqueue(diskFileInfo.absoluteFilePath());
            }

            if (m_stop)
                break;
        }

        // ## remove the files from the db in the fileInfosInDb hash now?
    }

    if (!diskFileInfos.isEmpty())
        updateMediaInfos(diskFileInfos);
}

void MediaScanner::refresh()
{
    QSqlQuery query(m_db);
    query.setForwardOnly(true);
    query.exec("SELECT path FROM directories");

    QStringList dirs;
    while (query.next()) {
        dirs << query.value(0).toString();
    }

    for (int i = 0; i < dirs.count(); i++) {
        if (!m_stop)
            scan(dirs[i]);
    }
}

