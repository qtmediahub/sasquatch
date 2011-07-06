#ifndef MEDIAPARSER_H
#define MEDIAPARSER_H

#include <QFileInfo>
#include <QList>
#include "mediascanner.h"
#include "global.h"

class QMH_EXPORT MediaParser : public QObject
{
    Q_OBJECT
public:
    MediaParser();
    virtual ~MediaParser() { }

    virtual QString type() const = 0;
    virtual bool canRead(const QFileInfo &info) const = 0;
    virtual QList<QSqlRecord> updateMediaInfos(const QList<QFileInfo> &fi, QSqlDatabase db) = 0;

    virtual QHash<QString, MediaScanner::FileInfo> findFilesByPath(const QString &path, QSqlDatabase db);

signals:
    void databaseUpdated(const QList<QSqlRecord> &records);
};

#endif // MEDIAPARSER_H

