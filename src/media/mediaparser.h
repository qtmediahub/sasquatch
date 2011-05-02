#ifndef MEDIAPARSER_H
#define MEDIAPARSER_H

#include <QFileInfo>
#include <QList>
#include "mediascanner.h"

class MediaParser
{
public:
    MediaParser(const QSqlDatabase &db);
    virtual ~MediaParser() { }

    virtual QString type() const = 0;
    virtual bool canRead(const QFileInfo &info) const = 0;
    virtual QList<QSqlRecord> updateMediaInfos(const QList<QFileInfo> &fi) = 0;

    virtual QHash<QString, MediaScanner::FileInfo> findFilesByPath(const QString &path);

protected:
    QSqlDatabase m_db;
};

class MusicParser : public MediaParser
{
public:
    MusicParser(const QSqlDatabase &db) : MediaParser(db) { }

    QString type() const;
    bool canRead(const QFileInfo &info) const;
    QList<QSqlRecord> updateMediaInfos(const QList<QFileInfo> &fi);
};

#endif // MEDIAPARSER_H

