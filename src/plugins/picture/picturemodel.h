#ifndef PICTUREMODEL_H
#define PICTUREMODEL_H

#include "../mediamodel.h"

struct PictureInfo : public MediaInfo
{
    PictureInfo() :  MediaInfo(MediaModel::File), orientation(TopLeft) { }

    QImage thumbnail;
    enum Orientation {
        TopLeft = 1,
        TopRight,
        BottomRight,
        BottomLeft,
        LeftTop,
        RightTop,
        RightBottom,
        LeftBottom
    };
    Orientation orientation;
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

