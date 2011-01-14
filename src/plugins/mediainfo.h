#ifndef MEDIAINFO_H
#define MEDIAINFO_H

#include <QObject>

#include "mediamodel.h"

class MediaInfo : public QObject
{
    Q_OBJECT
    Q_ENUMS(Status)

public:
    enum Status { NotSearched, Searching, Searched };

    MediaInfo(MediaModel::MediaInfoType type);
    ~MediaInfo();

    MediaInfo *parent;
    MediaModel::MediaInfoType type;
    QString filePath;
    QString name;
    Status status;
    QList<MediaInfo *> children;
    qint64 fileSize;
    QDateTime fileDateTime;
    QString thumbnail;
};

Q_DECLARE_METATYPE(MediaInfo*)

#endif // MEDIAINFO_H
