#include "videoparser.h"
#include "qmh-config.h"
#include <QtGui>
#include <QtSql>
#include "scopedtransaction.h"

#define DEBUG if (0) qDebug() << __PRETTY_FUNCTION__

bool VideoParser::canRead(const QFileInfo &info) const
{
    static QStringList supportedTypes = QStringList()
                << "avi"
                << "ogg"
                << "mp4"
                << "mpeg"
                << "mpg"
                << "mov"
                << "ogv"
                << "wmv"
                << "mkv";

    return supportedTypes.contains(info.suffix());
}

#ifdef THUMBNAIL_GSTREAMER

#include <gst/gst.h>

#define CAPS "video/x-raw-rgb,pixel-aspect-ratio=1/1,bpp=(int)24,depth=(int)24,endianness=(int)4321,red_mask=(int)0xff0000, green_mask=(int)0x00ff00, blue_mask=(int)0x0000ff"

static QImage generateThumbnailGstreamer(const QFileInfo &fileInfo)
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

        const int previewWidth = Config::value("thumbnail-size", "256").toInt();
        return image.width() <= previewWidth ? image: image.scaledToWidth(previewWidth, Qt::SmoothTransformation);
    } else {
        qDebug() << "could not make snapshot";
        return QImage();
    }
}
#endif

static QByteArray generateThumbnail(const QFileInfo &fileInfo)
{
    if (!Config::isEnabled("video-thumbnails", true))
        return QByteArray();

    QByteArray md5 = QCryptographicHash::hash("file://" + QFile::encodeName(fileInfo.absoluteFilePath()), QCryptographicHash::Md5).toHex();
    QFileInfo thumbnailInfo(MediaScanner::instance()->thumbnailPath() + md5 + ".png");
    if (!thumbnailInfo.exists())
        return md5;

#ifdef THUMBNAIL_GSTREAMER
    QImage image = generateThumbnailGstreamer(fileInfo);
    if (image.isNull())
        return QByteArray();

    image.save(thumbnailInfo.absoluteFilePath());
    return md5;
#else
    return QByteArray();
#endif
}

static QString determineTitle(const QFileInfo &info)
{
    QString title = info.baseName();
    title[0] = title[0].toUpper();
    return title;
}

// ## See if DELETE+INSERT is the best approach. Sqlite3 supports INSERT OR IGNORE which could aslo be used
// ## Also check other upsert methods
QList<QSqlRecord> VideoParser::updateMediaInfos(const QList<QFileInfo> &fis)
{
    QList<QSqlRecord> records;
    QSqlQuery query(m_db);
    ScopedTransaction transaction(m_db);

    foreach(const QFileInfo &fi, fis) {
        DEBUG << "Updating " << fi.absoluteFilePath();
        query.prepare("DELETE FROM video WHERE filepath=:filepath");
        query.bindValue(":filepath", fi.absoluteFilePath());
        if (!query.exec())
            DEBUG << query.lastError().text();

        if (!query.prepare("INSERT INTO video (filepath, title, thumbnail, directory, mtime, ctime, filesize) "
                           " VALUES (:filepath, :title, :thumbnail, :directory, :mtime, :ctime, :filesize)")) {
            DEBUG << query.lastError().text();
            return records;
        }

        query.bindValue(":filepath", fi.absoluteFilePath());
        query.bindValue(":title", determineTitle(fi));
        query.bindValue(":thumbnail", generateThumbnail(fi));

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

