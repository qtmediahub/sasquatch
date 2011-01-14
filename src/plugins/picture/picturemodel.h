#ifndef PICTUREMODEL_H
#define PICTUREMODEL_H

#include "../mediamodel.h"
#include "../mediainfo.h"
#include "exifreader.h"

class PictureInfo : public MediaInfo
{
public:
    PictureInfo() :  MediaInfo(MediaModel::File)
    { }

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
        ResolutionWidthRole,
        ResolutionHeightRole,
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
};

#endif // PICTUREMODEL_H

