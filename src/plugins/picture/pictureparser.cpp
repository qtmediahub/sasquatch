#include "pictureparser.h"
#include "exifreader.h"
#include "scopedtransaction.h"
#include "qmh-config.h"

#include <QtSql>
#include <QImageReader>

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

bool PictureParser::canRead(const QFileInfo &info) const
{
   QImageReader imageReader(info.absoluteFilePath());
   return imageReader.canRead();
}

static QString determineTitle(const ExifReader &reader, const QFileInfo &fi)
{
    QString title = reader.userComments().simplified();
    if (title.isEmpty())
        title = reader.imageDescription().simplified();
    if (title.isEmpty()) {
        title = fi.baseName();
        title[0] = title[0].toUpper();
    }

    return title;
}

static QByteArray determineThumbnail(const ExifReader &reader, const QFileInfo &info)
{
    QByteArray md5 = QCryptographicHash::hash("file://" + QFile::encodeName(info.absoluteFilePath()), QCryptographicHash::Md5).toHex();
    QFileInfo thumbnailInfo(MediaScanner::instance()->thumbnailPath() + md5 + ".png");
    if (thumbnailInfo.exists())
        return QUrl::fromLocalFile(thumbnailInfo.absoluteFilePath()).toEncoded();

    const int previewWidth = Config::value("thumbnail-size", "256").toInt();

    QImageReader imageReader(info.absoluteFilePath());
    QImage image = imageReader.read();
    QImage tmp = image.width() <= previewWidth ? image: image.scaledToWidth(previewWidth, Qt::SmoothTransformation);
    ExifReader::Orientation orientation = static_cast<ExifReader::Orientation>(reader.orientation());
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

    tmp.save(thumbnailInfo.absoluteFilePath());
    return QUrl::fromLocalFile(thumbnailInfo.absoluteFilePath()).toEncoded();;
}

// ## See if DELETE+INSERT is the best approach. Sqlite3 supports INSERT OR IGNORE which could aslo be used
// ## Also check other upsert methods
QList<QSqlRecord> PictureParser::updateMediaInfos(const QList<QFileInfo> &fis)
{
    QList<QSqlRecord> records;
    QSqlQuery query(m_db);
    ScopedTransaction transaction(m_db);

    foreach(const QFileInfo &fi, fis) {
        DEBUG << "Updating " << fi.absoluteFilePath();
        ExifReader reader(fi.absoluteFilePath());
        query.prepare("DELETE FROM picture WHERE filepath=:filepath");
        query.bindValue(":filepath", fi.absoluteFilePath());
        if (!query.exec())
            DEBUG << query.lastError().text();

        if (!query.prepare("INSERT INTO picture (filepath, title, thumbnail, comments, description, created, camera_model, camera_make, latitude, longitude, altitude, orientation, "
                           "aperture, focal_length, exposure_time, exposure_mode, white_balance, light_source, iso_speed, digital_zoom_ratio, flash_usage, color_space, directory, mtime, ctime, filesize) "
                           " VALUES (:filepath, :title, :thumbnail, :comments, :description, :created, :camera_model, :camera_make, :latitude, :longitude, :altitude, :orientation, "
                           ":aperture, :focal_length, :exposure_time, :exposure_mode, :white_balance, :light_source, :iso_speed, :digital_zoom_ratio, :flash_usage, :color_space, :directory, :mtime, :ctime, :filesize)")) {
            DEBUG << query.lastError().text();
            return records;
        }

        query.bindValue(":filepath", fi.absoluteFilePath());
        query.bindValue(":title", determineTitle(reader, fi));
        query.bindValue(":thumbnail", determineThumbnail(reader, fi));

        query.bindValue(":comments", reader.userComments());
        query.bindValue(":description", reader.imageDescription());
        query.bindValue(":created", reader.creationTime());
        query.bindValue(":camera_model", reader.cameraModel());
        query.bindValue(":camera_make", reader.cameraMake());

        bool ok1, ok2, ok3;
        double lat = reader.latitude(&ok1);
        double log = reader.longitude(&ok2);
        if (ok1 && ok2) {
            query.bindValue(":latitude", lat);
            query.bindValue(":longitude", log);
        }
        query.bindValue(":altitude", reader.altitude(&ok3));

        query.bindValue(":orientation", reader.orientation());
        query.bindValue(":aperture", reader.aperture());
        query.bindValue(":focal_length", reader.focalLength());
        query.bindValue(":exposure_mode", reader.exposureMode());
        query.bindValue(":white_balance", reader.whiteBalance());
        query.bindValue(":light_source", reader.lightSource());
        query.bindValue(":iso_speed", reader.isoSpeed());
        query.bindValue(":digital_zoom_ratio", reader.digitalZoomRatio());
        query.bindValue(":flash_usage", reader.flashUsage());
        query.bindValue(":exposure_time", reader.exposureTime());
        query.bindValue(":color_space", reader.colorSpace());

        query.bindValue(":directory", fi.absolutePath());
        query.bindValue(":mtime", fi.lastModified().toTime_t());
        query.bindValue(":ctime", fi.created().toTime_t());
        query.bindValue(":filesize", fi.size());

        if (!query.exec())
            DEBUG << query.lastError().text();
        
        QSqlRecord record;
        record.append(QSqlField("id", QVariant::Int));
        record.setValue(0, query.lastInsertId());
        QMap<QString, QVariant> boundValues = query.boundValues();
        for (QMap<QString, QVariant>::const_iterator it = boundValues.constBegin(); it != boundValues.constEnd(); ++it) {
            QString key = it.key().mid(1); // remove the ':'
            record.append(QSqlField(key, (QVariant::Type) it.value().type()));
            record.setValue(key, it.value());
        }
        records.append(record);
    }

    emit databaseUpdated(records);
    return records;
}


