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

#include "videoparser.h"
#include "qmh-config.h"
#include "libraryinfo.h"
#include <QtGui>
#include <QtSql>
#include "scopedtransaction.h"
#include "globalsettings.h"

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__


VideoParser::VideoParser(GlobalSettings *settings, QObject *parent) :
    MediaParser(settings, parent)
{
    m_supportedTypes << "avi"
                     << "ogg"
                     << "mp4"
                     << "mpeg"
                     << "mpg"
                     << "mov"
                     << "ogv"
                     << "wmv"
                     << "mkv"
                     << "ts";

    QStringList additionalExtensions = m_settings->value(GlobalSettings::ExtraVideoExtensions).toString().split(",", QString::SkipEmptyParts);

    if (!additionalExtensions.isEmpty()) {
        m_supportedTypes << additionalExtensions;
    }
}

bool VideoParser::canRead(const QFileInfo &info) const
{
    return m_supportedTypes.contains(info.suffix());
}

#ifdef THUMBNAIL_GSTREAMER

#include <gst/gst.h>

#define CAPS "video/x-raw-rgb,pixel-aspect-ratio=1/1,bpp=(int)24,depth=(int)24,endianness=(int)4321,red_mask=(int)0xff0000, green_mask=(int)0x00ff00, blue_mask=(int)0x0000ff"

static QImage generateThumbnailGstreamer(GlobalSettings *settings, const QFileInfo &fileInfo)
{
    GstElement *pipeline, *sink;
    gint width, height;
    GstBuffer *buffer;
    GError *error = NULL;
    gint64 duration, position;
    GstFormat format;
    GstStateChangeReturn ret;
    gboolean res;

    gst_init (NULL, NULL);

    QString descr = "uridecodebin uri=\"file://" + fileInfo.absoluteFilePath() + "\" ! ffmpegcolorspace ! videoscale ! appsink name=sink caps=\"" CAPS "\"";

    pipeline = gst_parse_launch (descr.toAscii(), &error);

    if (error != NULL) {
        qDebug() <<  "could not construct pipeline: " << error->message;
        g_error_free (error);
        return QImage();
    }


    sink = gst_bin_get_by_name (GST_BIN (pipeline), "sink");


    ret = gst_element_set_state (pipeline, GST_STATE_PAUSED);
    switch (ret) {
    case GST_STATE_CHANGE_FAILURE:
        qDebug() << "failed to play the file";
        return QImage();
    case GST_STATE_CHANGE_NO_PREROLL:

        qDebug() << "live sources not supported yet";
        return QImage();
    default:
        break;
    }
    ret = gst_element_get_state (pipeline, NULL, NULL, 5 * GST_SECOND);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qDebug() << "failed to play the file";
        return QImage();
    }

    /* get the duration */
    format = GST_FORMAT_TIME;
    gst_element_query_duration (pipeline, &format, &duration);

    if (duration != -1)
        position = duration * 0.5;
    else
        position = 1 * GST_SECOND;

    gst_element_seek_simple (pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, position);
    g_signal_emit_by_name (sink, "pull-preroll", &buffer, NULL);

    if (buffer) {
        GstCaps *caps;
        GstStructure *s;

        caps = GST_BUFFER_CAPS (buffer);
        if (!caps) {
            qDebug() <<  "could not get snapshot format";
            return QImage();
        }
        s = gst_caps_get_structure (caps, 0);

        res = gst_structure_get_int (s, "width", &width);
        res |= gst_structure_get_int (s, "height", &height);
        if (!res) {
            qDebug() << "could not get snapshot dimension";
            return QImage();
        }

        QImage image(GST_BUFFER_DATA (buffer), width, height, GST_ROUND_UP_4 (width * 3), QImage::Format_RGB888);

        gst_element_set_state (pipeline, GST_STATE_NULL);
        gst_object_unref (pipeline);

        const int previewWidth = settings->value(GlobalSettings::ThumbnailSize).toInt();
        return image.width() <= previewWidth ? image: image.scaledToWidth(previewWidth, Qt::SmoothTransformation);
    } else {
        qDebug() << "could not make snapshot";
        return QImage();
    }
}
#endif

static QByteArray generateThumbnail(GlobalSettings *settings, const QFileInfo &fileInfo)
{
    QByteArray md5 = QCryptographicHash::hash("file://" + QFile::encodeName(fileInfo.absoluteFilePath()), QCryptographicHash::Md5).toHex();
    QFileInfo thumbnailInfo(LibraryInfo::thumbnailPath(settings) + md5 + ".png");
    if (thumbnailInfo.exists())
        return QUrl::fromLocalFile(thumbnailInfo.absoluteFilePath()).toEncoded();

#ifdef THUMBNAIL_GSTREAMER
    QImage image = generateThumbnailGstreamer(settings, fileInfo);
    if (image.isNull())
        return QByteArray();

    image.save(thumbnailInfo.absoluteFilePath());
    return QUrl::fromLocalFile(thumbnailInfo.absoluteFilePath()).toEncoded();;
#else
    return QByteArray();
#endif
}

// clearly needs more advanced recognizer
static QString determineTitle(const QFileInfo &info)
{
    QString title = info.fileName();

    title.remove(title.lastIndexOf('.'), title.length());
    title.replace(QRegExp("[\\._\\-]"), " ");
    title.replace(QRegExp("xvid|rip|hdtv", Qt::CaseInsensitive), "" );
    title.simplified();
    title[0] = title[0].toUpper();

    return title;
}

static QPair<QString, QString> determineShowAndSeason(const QFileInfo &info, const QString &searchPath)
{
    QString show, season;
    QDir dir = info.dir();
    if (dir.absolutePath()== searchPath.midRef(0, searchPath.length()-1)) {
        QString baseName = info.fileName().mid(0, info.fileName().lastIndexOf('.'));
        QRegExp re("[^-)]*", Qt::CaseInsensitive);
        if (re.indexIn(baseName) == -1) {
            show = baseName;
        } else {
            show = re.cap(0);
        }
    } else {
        QDir parentDir = dir;
        if (!parentDir.cdUp() || parentDir.absolutePath() == searchPath.midRef(0, searchPath.length()-1)) {
            show = dir.dirName();
        } else {
            show = parentDir.dirName();
            season = dir.dirName();
        }
    }
    return qMakePair(show, season);
}

// ## See if DELETE+INSERT is the best approach. Sqlite3 supports INSERT OR IGNORE which could aslo be used
// ## Also check other upsert methods
QList<QSqlRecord> VideoParser::updateMediaInfos(const QList<QFileInfo> &fis, const QString &searchPath, QSqlDatabase db)
{
    Q_UNUSED(searchPath);
    QList<QSqlRecord> records;
    QSqlQuery query(db);
    ScopedTransaction transaction(db);

    foreach(const QFileInfo &fi, fis) {
        DEBUG << "Updating " << fi.absoluteFilePath();
        query.prepare("DELETE FROM video WHERE filepath=:filepath");
        query.bindValue(":filepath", fi.absoluteFilePath());
        if (!query.exec())
            qWarning() << query.lastError().text();

        if (!query.prepare("INSERT INTO video (filepath, title, thumbnail, uri, directory, mtime, ctime, filesize, show, season) "
                           " VALUES (:filepath, :title, :thumbnail, :uri, :directory, :mtime, :ctime, :filesize, :show, :season)")) {
            qWarning() << query.lastError().text();
            return records;
        }

        query.bindValue(":filepath", fi.absoluteFilePath());
        query.bindValue(":title", determineTitle(fi));
        query.bindValue(":thumbnail", generateThumbnail(m_settings, fi));
        query.bindValue(":uri", QUrl::fromLocalFile(fi.absoluteFilePath()).toEncoded());

        query.bindValue(":directory", fi.absolutePath() + '/');
        query.bindValue(":mtime", fi.lastModified().toTime_t());
        query.bindValue(":ctime", fi.created().toTime_t());
        query.bindValue(":filesize", fi.size());

        QPair<QString, QString> showAndSeason = determineShowAndSeason(fi, searchPath);
        query.bindValue(":show", showAndSeason.first);
        query.bindValue(":season", showAndSeason.second);

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

