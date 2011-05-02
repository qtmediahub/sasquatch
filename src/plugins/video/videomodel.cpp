/****************************************************************************
 *
 * This file is part of the QtMediaHub project on http://www.gitorious.org.
 *
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).*
 * All rights reserved.
 *
 * Contact:  Nokia Corporation (qt-info@nokia.com)**
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 *
 * ****************************************************************************/

#include "videomodel.h"

#include <QFileInfo>
#include <QProcess>

#include "qmh-config.h"

VideoModel::VideoModel(QObject *parent)
    : MediaModel(MediaModel::Video, parent)
{
    qRegisterMetaType<VideoInfo *>("VideoInfo *");

    QHash<int, QByteArray> roleNames = MediaModel::roleNames();
    roleNames[LengthRole] = "length";
    setRoleNames(roleNames);
}

VideoModel::~VideoModel()
{
}

QVariant VideoModel::data(MediaInfo *mediaInfo, int role) const
{
    VideoInfo *info = static_cast<VideoInfo *>(mediaInfo);


    if (role == Qt::DisplayRole) {
        return info->name;
    } else if (role == LengthRole) {
        return info->length;
    } else {
        return QVariant();
    }
}

static bool generateThumbnailMplayerProcess(const QFileInfo &fileInfo, const QFileInfo &thumbnailInfo)
{
    QDir dir;
    QFileInfo tmp("/tmp/00000001.png");
    if (tmp.exists())
        dir.remove(tmp.filePath());

    QString program = "mplayer";
    QStringList arguments;
    arguments << "-ss" << "60";
    arguments << "-nosound";
    arguments << "-vo" << "png";
    arguments << "-frames" << "1";
    arguments << fileInfo.filePath();

    QProcess process;
    process.setWorkingDirectory(tmp.path());
    process.start(program, arguments);
    process.waitForFinished();

    if (tmp.exists()) {
        dir.rename(tmp.filePath(), thumbnailInfo.filePath());
        dir.remove(tmp.filePath());
        return true;
    }

    return false;
}

static bool generateThumbnailGstreamerProcess(const QFileInfo &fileInfo, const QFileInfo &thumbnailInfo)
{
    QString program = "gst-launch";
    QStringList arguments;
    arguments << "uridecodebin" << "uri=file://" + fileInfo.filePath();
    arguments << "!";
    arguments << "ffmpegcolorspace";
    arguments << "!";
    arguments << "videoscale";
    arguments << "!";
    arguments << "pngenc" << "snapshot=true";
    arguments << "!";
    arguments << "filesink" << "location=" + thumbnailInfo.filePath();

    qDebug() << "using gstreamer" << program << arguments;

    QProcess process;
    process.start(program, arguments);
    process.waitForFinished();

    if (thumbnailInfo.exists())
        return true;

    return false;
}

#ifdef THUMBNAIL_GSTREAMER

#include <gst/gst.h>

#define CAPS "video/x-raw-rgb,pixel-aspect-ratio=1/1,bpp=(int)24,depth=(int)24,endianness=(int)4321,red_mask=(int)0xff0000, green_mask=(int)0x00ff00, blue_mask=(int)0x0000ff"

static bool generateThumbnailGstreamer(const QFileInfo &fileInfo, const QFileInfo &thumbnailInfo)
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
        return false;
    }


    sink = gst_bin_get_by_name (GST_BIN (pipeline), "sink");


    ret = gst_element_set_state (pipeline, GST_STATE_PAUSED);
    switch (ret) {
    case GST_STATE_CHANGE_FAILURE:
        qDebug() << "failed to play the file";
        return false;
    case GST_STATE_CHANGE_NO_PREROLL:

        qDebug() << "live sources not supported yet";
        return false;
    default:
        break;
    }
    ret = gst_element_get_state (pipeline, NULL, NULL, 5 * GST_SECOND);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qDebug() << "failed to play the file";
        return false;
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
            return false;
        }
        s = gst_caps_get_structure (caps, 0);

        res = gst_structure_get_int (s, "width", &width);
        res |= gst_structure_get_int (s, "height", &height);
        if (!res) {
            qDebug() << "could not get snapshot dimension";
            return false;
        }

        QImage image(GST_BUFFER_DATA (buffer), width, height, GST_ROUND_UP_4 (width * 3), QImage::Format_RGB888);

        gst_element_set_state (pipeline, GST_STATE_NULL);
        gst_object_unref (pipeline);

        return image.save(thumbnailInfo.absoluteFilePath());

    } else {
        qDebug() << "could not make snapshot";
        return false;
    }
}
#endif

static bool generateThumbnail(const QFileInfo &fileInfo, const QFileInfo &thumbnailInfo)
{
    if (!Config::isEnabled("video-thumbnails", true))
        return false;
#ifdef THUMBNAIL_GSTREAMER
    return generateThumbnailGstreamer(fileInfo, thumbnailInfo);
#else
    if (generateThumbnailGstreamerProcess(fileInfo, thumbnailInfo))
        return true;
    else if (generateThumbnailMplayerProcess(fileInfo, thumbnailInfo))
        return true;
    else
        return false;
#endif
}

MediaInfo *VideoModel::readMediaInfo(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    //FIXME: query supported extensions from underlying media framework
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

    if (!fileInfo.exists() || !supportedTypes.contains(fileInfo.suffix()))
        return 0;

    VideoInfo *info = new VideoInfo(filePath);
    QFileInfo thumbnailInfo(info->thumbnailPath);

    if (!thumbnailInfo.exists() && !generateThumbnail(fileInfo, thumbnailInfo))
        info->thumbnailPath = themeResourcePath() + "DefaultVideo.png";

    return info;
}

