#ifndef PICTUREMODEL_H
#define PICTUREMODEL_H

#include "../mediamodel.h"

struct PictureInfo : public MediaInfo
{
    PictureInfo() :  MediaInfo(MediaModel::File) { }

    QImage thumbnail;
    // this really needs to be something else like an enum but i haven't been able to decode the exif
    // codes yet.
    QString orientation;
};

class PictureModel : public MediaModel
{
    Q_OBJECT
public:
    PictureModel(QObject *parent = 0);
    ~PictureModel();

    enum CustomRoles {
    };

    QVariant data(MediaInfo *mediaInfo, int role) const;
    MediaInfo *readMediaInfo(const QString &filePath); // called from thread
    QImage decoration(MediaInfo *mediaInfo) const;
};

#endif // PICTUREMODEL_H

