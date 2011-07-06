#ifndef PICTUREPARSER_H
#define PICTUREPARSER_H

#include <QFileInfo>
#include <QList>
#include "mediaparser.h"

class PictureParser : public MediaParser
{
    Q_OBJECT
public:
    PictureParser() { }
    ~PictureParser() { }

    QString type() const { return "picture"; }
    bool canRead(const QFileInfo &info) const;
    QList<QSqlRecord> updateMediaInfos(const QList<QFileInfo> &fi, QSqlDatabase db);
};

#endif // PICTUREPARSER_H

