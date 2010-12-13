#ifndef PICTUREMODEL_H
#define PICTUREMODEL_H

#include "../mediamodel.h"
#include "exifreader.h"

struct PictureInfo : public MediaInfo
{
    PictureInfo() :  MediaInfo(MediaModel::File)
    { }

    QImage thumbnail;
    QSize resolution;

    // Exif
    QMap<QString, QVariant> exifProperties;
};

class PictureModel : public MediaModel
{
    Q_OBJECT

public:
    PictureModel(QObject *parent = 0);
    ~PictureModel();

    enum CustomRoles {
        ResolutionRole = Qt::UserRole + 100,
        UserCommentsRole,
        ImageDescriptionRole,
        CreationTimeRole,
        CameraMakeRole,
        CameraModelRole,
        LatitudeRole,
        LongitudeRole,
        AltitudeRole,
        OrientationRole,
        ExifPropertiesRole
    };

    QVariant data(MediaInfo *mediaInfo, int role) const;
    MediaInfo *readMediaInfo(const QString &filePath); // called from thread
    QImage decoration(MediaInfo *mediaInfo) const;
};

#endif // PICTUREMODEL_H

