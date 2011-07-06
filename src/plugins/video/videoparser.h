#ifndef VIDEOPARSER_H
#define VIDEOPARSER_H

#include "mediaparser.h"

class VideoParser : public MediaParser
{
    Q_OBJECT
public:
    VideoParser() { }
    ~VideoParser() { }

    QString type() const { return "video"; }
    bool canRead(const QFileInfo &info) const;
    QList<QSqlRecord> updateMediaInfos(const QList<QFileInfo> &fi, QSqlDatabase db);
};

#endif // VIDEOPARSER_H

