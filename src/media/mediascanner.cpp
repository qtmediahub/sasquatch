#include "mediascanner.h"
#include <QtSql>
#include "scopedtransaction.h"
#include "mediaparser.h"
#include "backend.h"
#include "qmh-config.h"

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

const QString CONNECTION_NAME("MediaScanner");
const int BULK_LIMIT = 100;

MediaScanner *MediaScanner::s_instance = 0;

MediaScanner *MediaScanner::instance()
{
    if (!s_instance)
        s_instance = new MediaScanner;
    return s_instance;
}

MediaScanner::MediaScanner(QObject *parent)
    : QObject(parent), m_stop(false)
{
}

QString MediaScanner::thumbnailPath() const
{
    return QDir::homePath() + "/.thumbnails/qtmediahub/";
}

void MediaScanner::initialize()
{
    m_db = QSqlDatabase::cloneDatabase(Backend::instance()->mediaDatabase(), CONNECTION_NAME);
    if (!m_db.open())
        DEBUG << "Erorr opening database" << m_db.lastError().text();
    QSqlQuery query(m_db);
    //query.exec("PRAGMA synchronous=OFF"); // dangerous, can corrupt db
    //query.exec("PRAGMA journal_mode=WAL");
    query.exec("PRAGMA count_changes=OFF");

    // check if thumbnail folder exists
    QFileInfo thumbnailFolderInfo(thumbnailPath()); // TODO: make the path configureable
    if (!thumbnailFolderInfo.exists()) {
        QDir dir;
        dir.mkpath(thumbnailFolderInfo.absoluteFilePath());
    }
}

MediaScanner::~MediaScanner()
{
    QSqlDatabase::removeDatabase(CONNECTION_NAME);
}

void MediaScanner::addParser(MediaParser *parser)
{
    parser->setDatabase(m_db);
    m_parsers.insert(parser->type(), parser);
}

void MediaScanner::addSearchPath(const QString &type, const QString &_path, const QString &name)
{
    QString path = QFileInfo(_path).absoluteFilePath();
    if (path.endsWith('/'))
        path.chop(1);

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO directories (path, name, type) VALUES (:path, :name, :type)");
    query.bindValue(":path", path);
    query.bindValue(":name", name);
    query.bindValue(":type", type);
    if (!query.exec()) {
        m_errorString = query.lastError().text();
        DEBUG << m_errorString;
        return;
    }

    if (m_parsers.contains(type))
        scan(m_parsers.value(type), path);
}

void MediaScanner::scan(MediaParser *parser, const QString &path)
{
    QQueue<QString> dirQ;
    dirQ.enqueue(path);

    QList<QFileInfo> diskFileInfos;

    while (!dirQ.isEmpty() && !m_stop) {
        QString curdir = dirQ.dequeue();
        QFileInfoList fileInfosInDisk = QDir(curdir).entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot|QDir::NoSymLinks);
        QHash<QString, FileInfo> fileInfosInDb = parser->findFilesByPath(curdir);

        DEBUG << "Scanning " << curdir << fileInfosInDisk.count() << " files in disk and " << fileInfosInDb.count() << "in database";

        foreach(const QFileInfo &diskFileInfo, fileInfosInDisk) {
            FileInfo dbFileInfo = fileInfosInDb.take(diskFileInfo.absoluteFilePath());

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
                    QList<QSqlRecord> records = parser->updateMediaInfos(diskFileInfos);
                    diskFileInfos.clear();
                }
                DEBUG << diskFileInfo.absoluteFilePath() << " : added";
            } else if (diskFileInfo.isDir()) {
                dirQ.enqueue(diskFileInfo.absoluteFilePath());
            }

            if (m_stop)
                break;
        }

        usleep(Config::value("scanning-padding", 0)); // option to slow things down, because otherwise the disk gets thrashed and the ui becomes laggy
        // ## remove the files from the db in the fileInfosInDb hash now?
    }

    if (!diskFileInfos.isEmpty())
        parser->updateMediaInfos(diskFileInfos);
}

void MediaScanner::refresh()
{
    QSqlQuery query(m_db);
    query.setForwardOnly(true);
    query.exec("SELECT type, path FROM directories");

    QList<QPair<QString, QString> > dirs;
    while (query.next()) {
        dirs << qMakePair(query.value(0).toString(), query.value(1).toString());
    }

    for (int i = 0; i < dirs.count(); i++) {
        if (m_stop)
            break;
        if (!m_parsers.contains(dirs[i].first)) {
            DEBUG << "No registered parser for '" << dirs[i].first << "'";
            continue;
        }
        scan(m_parsers.value(dirs[i].first), dirs[i].second);
    }
}

