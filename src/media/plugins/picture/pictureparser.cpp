/****************************************************************************

This file is part of the QtMediaHub project on http://www.qtmediahub.com

Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Girish Ramakrishnan girish@forwardbias.in
Contact:  Nokia Corporation donald.carr@nokia.com
Contact:  Nokia Corporation johannes.zellner@nokia.com

You may use this file under the terms of the BSD license as follows:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include "pictureparser.h"
#include "exifreader.h"
#include "scopedtransaction.h"
#include "libraryinfo.h"
#include "globalsettings.h"

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

static QByteArray determineThumbnail(GlobalSettings *settings, const ExifReader &reader, const QFileInfo &info)
{
    QByteArray md5 = QCryptographicHash::hash("file://" + QFile::encodeName(info.absoluteFilePath()), QCryptographicHash::Md5).toHex();
    QFileInfo thumbnailInfo(LibraryInfo::thumbnailPath(settings) + md5 + ".png");
    if (thumbnailInfo.exists())
        return QUrl::fromLocalFile(thumbnailInfo.absoluteFilePath()).toEncoded();

    const int previewWidth = settings->value(GlobalSettings::ThumbnailSize).toInt();

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
QList<QSqlRecord> PictureParser::updateMediaInfos(const QList<QFileInfo> &fis, const QString &searchPath, QSqlDatabase db)
{
    Q_UNUSED(searchPath);
    QList<QSqlRecord> records;
    QSqlQuery query(db);
    ScopedTransaction transaction(db);

    foreach(const QFileInfo &fi, fis) {
        DEBUG << "Updating " << fi.absoluteFilePath();
        ExifReader reader(fi.absoluteFilePath());
        query.prepare("DELETE FROM picture WHERE filepath=:filepath");
        query.bindValue(":filepath", fi.absoluteFilePath());
        if (!query.exec())
            qWarning() << query.lastError().text();

        if (!query.prepare("INSERT INTO picture (filepath, title, thumbnail, year, month, comments, description, created, camera_model, camera_make, latitude, longitude, altitude, orientation, "
                           "aperture, focal_length, exposure_time, exposure_mode, white_balance, light_source, iso_speed, digital_zoom_ratio, flash_usage, color_space, directory, mtime, ctime, filesize) "
                           " VALUES (:filepath, :title, :thumbnail, :year, :month, :comments, :description, :created, :camera_model, :camera_make, :latitude, :longitude, :altitude, :orientation, "
                           ":aperture, :focal_length, :exposure_time, :exposure_mode, :white_balance, :light_source, :iso_speed, :digital_zoom_ratio, :flash_usage, :color_space, :directory, :mtime, :ctime, :filesize)")) {
            qWarning() << query.lastError().text();
            return records;
        }

        query.bindValue(":filepath", fi.absoluteFilePath());
        query.bindValue(":title", determineTitle(reader, fi));
        query.bindValue(":thumbnail", determineThumbnail(m_settings, reader, fi));

        QDateTime tmp = reader.creationTime();
        query.bindValue(":year", tmp.toString("yyyy").toInt());
        query.bindValue(":month", tmp.toString("MM").toInt());

        query.bindValue(":comments", reader.userComments());
        query.bindValue(":description", reader.imageDescription());
        query.bindValue(":created", tmp);
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

        query.bindValue(":directory", fi.absolutePath() + '/');
        query.bindValue(":mtime", fi.lastModified().toTime_t());
        query.bindValue(":ctime", fi.created().toTime_t());
        query.bindValue(":filesize", fi.size());

        if (!query.exec())
            qWarning() << query.lastError().text();
        
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


