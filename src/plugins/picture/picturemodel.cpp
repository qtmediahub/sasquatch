#include "picturemodel.h"

#include <QFile>
#include <QImageReader>

PictureModel::PictureModel(QObject *parent)
    : MediaModel(MediaModel::Picture, parent)
{
    QHash<int, QByteArray> roleNames = MediaModel::roleNames();
    setRoleNames(roleNames);
}

PictureModel::~PictureModel()
{
}

QVariant PictureModel::data(MediaInfo *mediaInfo, int role) const
{
    PictureInfo *pictureInfo = static_cast<PictureInfo *>(mediaInfo);
    if (role == Qt::DisplayRole)
        return mediaInfo->name;
    else if (role == UserCommentsRole)
        return pictureInfo->userComments;
    else if (role == ImageDescriptionRole)
        return pictureInfo->imageDescription;
    else if (role == CreationTimeRole)
        return pictureInfo->creationTime;
    else if (role == CameraMakeRole)
        return pictureInfo->cameraMake;
    else if (role == CameraModelRole)
        return pictureInfo->cameraModel;
    else if (role == LatitudeRole)
        return pictureInfo->latitude;
    else if (role == LongitudeRole)
        return pictureInfo->longitude;
    else if (role == AltitudeRole)
        return pictureInfo->altitude;
    else if (role == OrientationRole)
        return pictureInfo->orientation;

    return QVariant();
}

QImage PictureModel::decoration(MediaInfo *info) const
{
    return (static_cast<PictureInfo *>(info))->thumbnail;
}

MediaInfo *PictureModel::readMediaInfo(const QString &filePath)
{
    QImageReader imageReader(filePath);
    if (!imageReader.canRead())
        return 0;

    QImage image = imageReader.read();
    if (image.isNull())
        return 0;

    PictureInfo *info = new PictureInfo;
    info->thumbnail = image;

    ExifReader exifReader(filePath);
    info->userComments = exifReader.userComments();
    info->imageDescription = exifReader.imageDescription();
    info->creationTime = exifReader.creationTime();
    info->cameraModel = exifReader.cameraModel();
    info->cameraMake = exifReader.cameraMake();

    bool ok1, ok2, ok3;
    info->latitude = exifReader.latitude(&ok1);
    info->longitude = exifReader.longitude(&ok2);
    info->hasGeolocation = ok1 && ok2;
    info->altitude = exifReader.altitude(&ok3);

    info->orientation = exifReader.orientation();

    info->aperture = exifReader.aperture();
    info->focalLength = exifReader.focalLength();
    info->exposureMode = exifReader.exposureMode();
    info->whiteBalance = exifReader.whiteBalance();
    info->lightSource = exifReader.lightSource();
    info->isoSpeed = exifReader.isoSpeed();
    info->digitalZoomRatio = exifReader.digitalZoomRatio();
    info->flashUsage = exifReader.flashUsage();
    info->exposureTime = exifReader.exposureTime();
    info->colorSpace = exifReader.colorSpace();

    return info;
}

