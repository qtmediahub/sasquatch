#ifndef PICTUREMODEL_H
#define PICTUREMODEL_H

#include "../mediamodel.h"
#include "exifreader.h"

struct PictureInfo : public MediaInfo
{
    PictureInfo() :  MediaInfo(MediaModel::File), latitude(0), longitude(0), altitude(0),
                     hasGeolocation(false), orientation(ExifReader::NoOrientation)
    { }

    QImage thumbnail;

    // Exif
    QString userComments;
    QString imageDescription;
    QDateTime creationTime;
    QString cameraModel;
    QString cameraMake;

    double latitude;
    double longitude;
    double altitude;

    bool hasGeolocation;

    ExifReader::Orientation orientation;

    QString aperture;
    QString focalLength;
    QString exposureMode;
    QString whiteBalance;
    QString lightSource;
    QString isoSpeed;
    QString digitalZoomRatio;
    QString flashUsage;
    QString exposureTime;
    QString colorSpace;
};

class PictureModel : public MediaModel
{
    Q_OBJECT
public:
    PictureModel(QObject *parent = 0);
    ~PictureModel();

    enum CustomRoles {
        UserCommentsRole,
        ImageDescriptionRole,
        CreationTimeRole,
        CameraMakeRole,
        CameraModelRole,
        LatitudeRole,
        LongitudeRole,
        AltitudeRole,
        OrientationRole
    };

    QVariant data(MediaInfo *mediaInfo, int role) const;
    MediaInfo *readMediaInfo(const QString &filePath); // called from thread
    QImage decoration(MediaInfo *mediaInfo) const;
};

#endif // PICTUREMODEL_H

