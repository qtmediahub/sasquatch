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

#include "mediascanner.h"
#include <QtSql>
#include "scopedtransaction.h"
#include "mediaparser.h"
#include "qmh-config.h"

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__
#define WARNING qWarning() << __PRETTY_FUNCTION__

const QString CONNECTION_NAME("MediaScanner");
const int BULK_LIMIT = 100;

class MediaScannerWorker : public QObject
{
    Q_OBJECT
public:
    MediaScannerWorker(MediaScanner *scanner);
    ~MediaScannerWorker();

public slots:
    void initializeDatabase(const QSqlDatabase &db);
    void addParser(MediaParser *parser);
    void addSearchPath(const QString &type, const QString &_path, const QString &name);
    void removeSearchPath(const QString &type, const QString &path);
    void refresh(const QString &type);
    void stop() { m_stop = true; }

signals:
    void scanPathChanged(const QString &path);

private:
    void scan(MediaParser *parser, const QString &path);

    MediaScanner *m_scanner;
    QSqlDatabase m_db;
    QHash<QString, MediaParser *> m_parsers;
    volatile bool m_stop;
};

Q_DECLARE_METATYPE(QSqlDatabase) // ## may not be the best place...

MediaScanner::MediaScanner(const QSqlDatabase &db, QObject *parent)
    : QObject(parent), m_db(db)
{
    qRegisterMetaType<MediaParser *>();
    qRegisterMetaType<QSqlDatabase>();

    m_workerThread = new QThread(this);
    m_workerThread->start();
    m_worker = new MediaScannerWorker(this);
    connect(m_worker, SIGNAL(scanPathChanged(QString)), this, SLOT(handleScanPathChanged(QString)));
    m_worker->moveToThread(m_workerThread);
    QMetaObject::invokeMethod(m_worker, "initializeDatabase", Qt::QueuedConnection, Q_ARG(QSqlDatabase, db));
}

MediaScanner::~MediaScanner()
{
    m_worker->deleteLater();
    m_workerThread->quit();
    m_workerThread->wait();
}

void MediaScanner::addParser(MediaParser *parser)
{
    QMetaObject::invokeMethod(m_worker, "addParser", Qt::QueuedConnection, Q_ARG(MediaParser *, parser));
}

void MediaScanner::addSearchPath(const QString &type, const QString &path, const QString &name)
{
    QMetaObject::invokeMethod(m_worker, "addSearchPath", Qt::QueuedConnection, Q_ARG(QString, type), 
                             Q_ARG(QString, path), Q_ARG(QString, name));
}

void MediaScanner::removeSearchPath(const QString &type, const QString &path)
{
    QMetaObject::invokeMethod(m_worker, "removeSearchPath", Q_ARG(QString, type), Q_ARG(QString, path));
}

QStringList MediaScanner::searchPaths(const QString &type) const
{
    QSqlQuery query(m_db);
    query.setForwardOnly(true);
    query.prepare("SELECT path FROM directories WHERE type = :1");
    query.addBindValue(type);
    query.exec();

    QStringList paths;
    while (query.next()) {
        paths.append(query.value(0).toString());
    }
    return paths;
}

void MediaScanner::refresh(const QString &type)
{
    QMetaObject::invokeMethod(m_worker, "refresh", Q_ARG(QString, type));
}

void MediaScanner::stop()
{
    QMetaObject::invokeMethod(m_worker, "stop");
}

void MediaScanner::handleScanPathChanged(const QString &scanPath)
{
    m_currentScanPath = scanPath;
    emit currentScanPathChanged();
}

MediaScannerWorker::MediaScannerWorker(MediaScanner *scanner)
    : m_scanner(scanner), m_stop(false)
{
}

MediaScannerWorker::~MediaScannerWorker()
{
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(CONNECTION_NAME);
}

void MediaScannerWorker::initializeDatabase(const QSqlDatabase &db)
{
    m_db = QSqlDatabase::cloneDatabase(db, CONNECTION_NAME);
    if (!m_db.open())
        WARNING << "Erorr opening database" << m_db.lastError().text();
    QSqlQuery query(m_db);
    //query.exec("PRAGMA synchronous=OFF"); // dangerous, can corrupt db
    //query.exec("PRAGMA journal_mode=WAL");
    query.exec("PRAGMA count_changes=OFF");
}

void MediaScannerWorker::addParser(MediaParser *parser)
{
    m_parsers.insert(parser->type(), parser);
    refresh(parser->type());
}

void MediaScannerWorker::addSearchPath(const QString &type, const QString &_path, const QString &name)
{
    QString path = QFileInfo(_path).absoluteFilePath();
    if (!path.endsWith('/'))
        path.append('/');

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO directories (path, name, type) VALUES (:path, :name, :type)");
    query.bindValue(":path", path);
    query.bindValue(":name", name);
    query.bindValue(":type", type);
    if (!query.exec()) {
        WARNING << query.lastError().text();
        return;
    }

    emit m_scanner->searchPathAdded(type, path, name);

    if (m_parsers.contains(type)) {
        emit m_scanner->scanStarted(type);
        scan(m_parsers.value(type), path);
        emit m_scanner->scanFinished(type);
    }
}

void MediaScannerWorker::removeSearchPath(const QString &type, const QString &_path)
{
    QString path = QFileInfo(_path).absoluteFilePath();
    if (!path.endsWith('/'))
        path.append('/');

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM directories WHERE type=:type AND path=:path");
    query.bindValue(":path", path);
    query.bindValue(":type", type);
    if (!query.exec()) {
        WARNING << "Removing directory " << query.lastError().text();
        return;
    }

    query = QSqlQuery(m_db);
    query.prepare(QString("DELETE FROM %1 WHERE directory LIKE :path").arg(type));
    query.bindValue(":path", path + '%');
    if (!query.exec()) {
        WARNING << "Removing data " << query.lastError().text();
        return;
    }

    emit m_scanner->searchPathRemoved(type, path);
}

void MediaScannerWorker::scan(MediaParser *parser, const QString &path)
{
    QQueue<QString> dirQ;
    dirQ.enqueue(path);

    QList<QFileInfo> diskFileInfos;

    QSet<qint64> fileIds = parser->fileIdsInPath(path, m_db);

    while (!dirQ.isEmpty() && !m_stop) {
        QString curdir = dirQ.dequeue();
        QFileInfoList fileInfosInDisk = QDir(curdir).entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot|QDir::NoSymLinks);
        QHash<QString, MediaScanner::FileInfo> fileInfosInDb = parser->topLevelFilesInPath(curdir, m_db);

        emit scanPathChanged(curdir);

        DEBUG << "Scanning " << curdir << fileInfosInDisk.count() << " files in disk and " << fileInfosInDb.count() << "in database";

        foreach(const QFileInfo &diskFileInfo, fileInfosInDisk) {
            MediaScanner::FileInfo dbFileInfo = fileInfosInDb.take(diskFileInfo.absoluteFilePath());
            fileIds.remove(dbFileInfo.rowid);

            if (diskFileInfo.isFile()) {
                if (!parser->canRead(diskFileInfo))
                    continue;

                if (diskFileInfo.lastModified().toTime_t() == dbFileInfo.mtime
                    && diskFileInfo.created().toTime_t() == dbFileInfo.ctime
                    && diskFileInfo.size() == dbFileInfo.size) {
                    DEBUG << diskFileInfo.absoluteFilePath() << " : no change";
                    continue;
                }

                diskFileInfos.append(diskFileInfo);
                if (diskFileInfos.count() > BULK_LIMIT) {
                    QList<QSqlRecord> records = parser->updateMediaInfos(diskFileInfos, m_db);
                    diskFileInfos.clear();
                }
                DEBUG << diskFileInfo.absoluteFilePath() << " : added";
            } else if (diskFileInfo.isDir()) {
                dirQ.enqueue(diskFileInfo.absoluteFilePath() + '/');
            }

            if (m_stop)
                break;
        }

        if (!m_stop) {
            usleep(Config::value("scan-delay", 0)); // option to slow things down, because otherwise the disk gets thrashed and the ui becomes laggy
        }
    }

    if (!diskFileInfos.isEmpty())
        parser->updateMediaInfos(diskFileInfos, m_db);

    DEBUG << "Removing " << fileIds;
    parser->removeFiles(fileIds, m_db);

    emit scanPathChanged(QString());
}

void MediaScannerWorker::refresh(const QString &type)
{
    DEBUG << "Refreshing type" << type;

    QSqlQuery query(m_db);
    query.setForwardOnly(true);
    if (type.isEmpty()) {
        query.exec("SELECT type, path FROM directories ORDER BY type");
    } else {
        query.prepare("SELECT type, path FROM directories WHERE type = :1");
        query.addBindValue(type);
        query.exec();
    }

    QString lastType;
    MediaParser *parser = 0;
    while (query.next()) {
        QString type = query.value(0).toString();
        QString path = query.value(1).toString();

        const bool typeChanged = lastType != type;

        if (typeChanged) {
            if (!lastType.isEmpty())
                emit m_scanner->scanFinished(lastType);

            parser = m_parsers.value(type);
            if (!parser) {
                WARNING << "No parser found for type '" << type << "'";
                continue;
            }
            emit m_scanner->scanStarted(type);
            lastType = type;
        }

        scan(parser, path);
    }

    if (!lastType.isEmpty())
        emit m_scanner->scanFinished(lastType);
}

#include "mediascanner.moc"

