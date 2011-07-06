#ifndef MUSICPARSER_H
#define MUSICPARSER_H

#include <QFileInfo>
#include <QList>
#include "mediaparser.h"

class MusicParser : public MediaParser
{
    Q_OBJECT
public:
    MusicParser() { }
    ~MusicParser() { }

    QString type() const { return "music"; }
    bool canRead(const QFileInfo &info) const;
    QList<QSqlRecord> updateMediaInfos(const QList<QFileInfo> &fi, QSqlDatabase db);
};

#endif // MUSICPARSER_H

