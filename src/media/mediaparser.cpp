#include "mediaparser.h"
#include "tagreader.h"
#include "scopedtransaction.h"
#include <QtSql>

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

MediaParser::MediaParser()
{
}

QHash<QString, MediaScanner::FileInfo> MediaParser::findFilesByPath(const QString &path)
{
    QHash<QString, MediaScanner::FileInfo> hash;
    QSqlQuery query(m_db);
    query.setForwardOnly(true);
    query.prepare("SELECT filepath, mtime, ctime, filesize FROM :type WHERE directory=:path");
    query.bindValue(":type", type());
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

