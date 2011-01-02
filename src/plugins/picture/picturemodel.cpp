#include "picturemodel.h"

#include <QFile>
#include <QImageReader>
#include <QModelIndex>
#include <QDeclarativePropertyMap>
#include <QFileInfo>

PictureModel::PictureModel(QObject *parent)
    : MediaModel(MediaModel::Picture, parent)
{
    QHash<int, QByteArray> roleNames = MediaModel::roleNames();
    roleNames[ResolutionRole] = "resolution";
    roleNames[UserCommentsRole] = "userComments";
    roleNames[ImageDescriptionRole] = "imageDescription";
    roleNames[CreationTimeRole] = "creationTime";
    roleNames[CameraMakeRole] = "cameraMake";
    roleNames[CameraModelRole] = "cameraModel";
    roleNames[LatitudeRole] = "latitude";
    roleNames[LongitudeRole] = "longitude";
    roleNames[AltitudeRole] = "altitude";
    roleNames[OrientationRole] = "orientation";
    roleNames[ExifPropertiesRole] = "exifProperties";
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
    if (role == ResolutionRole)
        return pictureInfo->resolution;
    else if (role == UserCommentsRole)
        return pictureInfo->exifProperties["userComments"];
    else if (role == ImageDescriptionRole)
        return pictureInfo->exifProperties["imageDescription"];
    else if (role == CreationTimeRole)
        return pictureInfo->exifProperties["creationTime"];
    else if (role == CameraMakeRole)
        return pictureInfo->exifProperties["cameraMake"];
    else if (role == CameraModelRole)
        return pictureInfo->exifProperties["cameraModel"];
    else if (role == LatitudeRole)
        return pictureInfo->exifProperties["latitude"];
    else if (role == LongitudeRole)
        return pictureInfo->exifProperties["longitude"];
    else if (role == AltitudeRole)
        return pictureInfo->exifProperties["altitude"];
    else if (role == OrientationRole)
        return pictureInfo->exifProperties["orientation"];
    else if (role == ExifPropertiesRole) {
        return pictureInfo->exifProperties;
    }
    return QVariant();
}

MediaInfo *PictureModel::readMediaInfo(const QString &filePath)
{
    QImageReader imageReader(filePath);
    if (!imageReader.canRead())
        return 0;

    PictureInfo *info = new PictureInfo;
    info->resolution = imageReader.size(); // ## only Qt's image readers support this!

    ExifReader exifReader(filePath);
    info->exifProperties["userComments"] = exifReader.userComments();
    info->exifProperties["imageDescription"] = exifReader.imageDescription();
    info->exifProperties["creationTime"] = exifReader.creationTime();
    info->exifProperties["cameraModel"] = exifReader.cameraModel();
    info->exifProperties["cameraMake"] = exifReader.cameraMake();

    bool ok1, ok2, ok3;
    double lat = exifReader.latitude(&ok1);
    double log = exifReader.longitude(&ok2);
    if (ok1 && ok2) {
        info->exifProperties["latitude"] = lat;
        info->exifProperties["longitude"] = log;
    }
    info->exifProperties["altitude"] = exifReader.altitude(&ok3);

    info->exifProperties["orientation"] = exifReader.orientation();
    info->exifProperties["aperture"] = exifReader.aperture();
    info->exifProperties["focalLength"] = exifReader.focalLength();
    info->exifProperties["exposureMode"] = exifReader.exposureMode();
    info->exifProperties["whiteBalance"] = exifReader.whiteBalance();
    info->exifProperties["lightSource"] = exifReader.lightSource();
    info->exifProperties["isoSpeed"] = exifReader.isoSpeed();
    info->exifProperties["digitalZoomRatio"] = exifReader.digitalZoomRatio();
    info->exifProperties["flashUsage"] = exifReader.flashUsage();
    info->exifProperties["exposureTime"] = exifReader.exposureTime();
    info->exifProperties["colorSpace"] = exifReader.colorSpace();

    // check if we already have a local cover art for this file
    QFileInfo fileInfo(filePath);
    QFileInfo thumbnailInfo = generateThumbnailFileInfo(fileInfo);

    if (thumbnailInfo.exists()) {
        info->thumbnail = thumbnailInfo.filePath();
    } else {
        QImage image = imageReader.read();
        QImage tmp = image.width() <= previewWidth() ? image: image.scaledToWidth(previewWidth(), Qt::SmoothTransformation);
        ExifReader::Orientation orientation = static_cast<ExifReader::Orientation>(info->exifProperties["orientation"].toInt());
        QTransform transform;
        switch (orientation) {
        case ExifReader::FlipHorizontal: tmp = tmp.mirrored(true, false); tmp = tmp.transformed(transform); break;
        case ExifReader::Rotate180: transform.rotate(180); tmp = tmp.transformed(transform); break;
        case ExifReader::FlipVertical: tmp = tmp.mirrored(false, true); tmp = tmp.transformed(transform); break;
        case ExifReader::Transpose: tmp = tmp.mirrored(true, true); break;
        case ExifReader::Rotate90: transform.rotate(90); tmp = tmp.transformed(transform); break;
        case ExifReader::Transverse: /* wth */ break;
        case ExifReader::Rotate270: transform.rotate(270); tmp = tmp.transformed(transform); break;
        default: break;
        }

        if(tmp.save(thumbnailInfo.filePath()))
            info->thumbnail = thumbnailInfo.filePath();
        else
            info->thumbnail = themeResourcePath() + "/media/DefaultPicture.png";
    }

    return info;
}

