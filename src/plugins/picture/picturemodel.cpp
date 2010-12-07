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
    if (role == Qt::DisplayRole)
        return mediaInfo->name;
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

    return info;
}

