#include "picturemodel.h"

#include <QFile>
#include <QImageReader>

#include <libexif/exif-loader.h>
#include <libexif/exif-data.h>

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

    // get orientation
    ExifLoader *loader = exif_loader_new();

    exif_loader_write_file(loader, filePath.toLatin1().constData());
    ExifData *data = exif_loader_get_data(loader);
    exif_loader_unref(loader); // deleting it

    ExifEntry *entry = exif_data_get_entry(data, EXIF_TAG_ORIENTATION);
    info->orientation = (PictureInfo::Orientation)(*(entry->data));
    if (info->orientation == 0) {
        // default value should be top left
        info->orientation = PictureInfo::TopLeft;
    }
    exif_entry_unref(entry);
    exif_data_unref(data);

    return info;
}

