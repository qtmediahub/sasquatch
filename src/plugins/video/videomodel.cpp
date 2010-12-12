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
#include <QDir>
#include <QCryptographicHash>
#include <QProcess>

VideoModel::VideoModel(QObject *parent)
    : MediaModel(MediaModel::Video, parent)
{
    qRegisterMetaType<VideoInfo *>("VideoInfo *");

    QHash<int, QByteArray> roleNames = MediaModel::roleNames();
    roleNames[LengthRole] = "length";
    roleNames[ThumbnailRole] = "thumbnail";
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
    } else if (role == ThumbnailRole) {
        return info->thumbnail;
    } else {
        return QVariant();
    }
}

QImage VideoModel::decoration(MediaInfo *mediaInfo) const
{
    VideoInfo *info = static_cast<VideoInfo *>(mediaInfo);

    QImage img = QImage(info->thumbnail);
    if (img.isNull())
        img = QImage(themeResourcePath() + "/media/DefaultVideo.png");

    return img;
}

static bool generateThumbnail(const QFileInfo &fileInfo, const QFileInfo &thumbnailInfo)
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

MediaInfo *VideoModel::readMediaInfo(const QString &filePath)
{
    QFileInfo thumbnailFolderInfo(QDir::homePath() + "/.thumbnails/qtmediahub/");
    if (!thumbnailFolderInfo.exists()) {
        QDir dir;
        dir.mkpath(thumbnailFolderInfo.absoluteFilePath());
    }

    QFileInfo fileInfo(filePath);
    QStringList supportedTypes;
    supportedTypes << "avi" << "ogg" << "mp4" << "mpeg" << "mpg" << "mov";

    if (!fileInfo.exists() || !supportedTypes.contains(fileInfo.suffix()))
        return 0;

    VideoInfo *info = new VideoInfo;

    QString md5 = QCryptographicHash::hash(QString("file://" + fileInfo.absoluteFilePath()).toUtf8(), QCryptographicHash::Md5).toHex();
    QFileInfo thumbnailInfo(thumbnailFolderInfo.filePath() + md5 + ".png");

    if (thumbnailInfo.exists() || generateThumbnail(fileInfo, thumbnailInfo))
        info->thumbnail = thumbnailInfo.filePath();

    return info;
}

