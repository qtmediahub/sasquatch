#ifndef MEDIAPARSER_H
#define MEDIAPARSER_H

#include <QFileInfo>
#include <QList>
#include "mediascanner.h"

class MediaParser : public QObject
{
    Q_OBJECT
public:
    MediaParser();
    virtual ~MediaParser() { }

    void setDatabase(const QSqlDatabase &db) { m_db = db; }

    virtual QString type() const = 0;
    virtual bool canRead(const QFileInfo &info) const = 0;
    virtual QList<QSqlRecord> updateMediaInfos(const QList<QFileInfo> &fi) = 0;

    virtual QHash<QString, MediaScanner::FileInfo> findFilesByPath(const QString &path);

signals:
    void databaseUpdated(const QList<QSqlRecord> &records);

protected:
    QSqlDatabase m_db;
};

#endif // MEDIAPARSER_H

