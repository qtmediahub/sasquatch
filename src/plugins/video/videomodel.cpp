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
#include <QDirIterator>
#include <QThreadPool>
#include <QTimer>
#include <QtDebug>
#include <QProcess>
#include <QCryptographicHash>


VideoModel::VideoModel(QObject *parent)
    : QAbstractItemModel(parent),
      m_thread(0),
      m_nowSearching(-1)
{
    qRegisterMetaType<VideoInfo>("VideoInfo");

    QHash<int, QByteArray> roleNames;
    roleNames[Qt::DisplayRole] = "display";
    roleNames[DecorationUrlRole] = "decorationUrl";
    roleNames[LengthRole] = "length";
    roleNames[SubtitlesRole] = "subtitles";
    roleNames[ChannelsRole] = "channels";
    roleNames[FilePathRole] = "filePath";
    roleNames[FileNameRole] = "fileName";
    setRoleNames(roleNames);

    Data *data = new Data(QString("AddNewSource"), tr("Add new source"));
    m_data.append(data);
}

VideoModel::~VideoModel()
{
    for (int i = 0; i < m_data.count(); i++)
        qDeleteAll(m_data[i]->videoInfos);
    qDeleteAll(m_data);

    // FIXME: Wait until thread is dead
    delete m_thread;
}

void VideoModel::startSearchThread()
{
    if (m_nowSearching != -1)
        return; // already searching some directory

    int i;
    for (i = 0; i < m_data.count()-1; i++) { // leave out the last item
        if (m_data[i]->status == Data::NotSearched)
            break;
    }
    if (i == m_data.count()-1) {
        m_nowSearching = -1;
        return; // all searched
    }

    Q_ASSERT(!m_thread);
    m_thread = new VideoModelThread(this, i, m_data[i]->searchPath);
    m_data[i]->status = Data::Searching;
    m_nowSearching = i;
    connect(m_thread, SIGNAL(videoFound(int, VideoInfo, QImage)), this, SLOT(addVideo(int, VideoInfo, QImage)));
    connect(m_thread, SIGNAL(finished()), this, SLOT(searchThreadFinished()));
    QThreadPool::globalInstance()->start(m_thread);
}

void VideoModel::searchThreadFinished()
{
    Q_ASSERT(m_nowSearching != -1);
    Q_ASSERT(m_thread);
    m_data[m_nowSearching]->status = Data::Searched;
    m_nowSearching = -1;
    // TODO will leak but currently crashes....
//    delete m_thread;
    m_thread = 0;

    startSearchThread();
}

void VideoModel::stopSearchThread()
{
    m_thread->stop();
}

void VideoModel::addSearchPath(const QString &path, const QString &name)
{
    beginInsertRows(QModelIndex(), m_data.count()-1, m_data.count()-1);
    Data *data = new Data(path, name);
    VideoInfo *mi = new VideoInfo;
    mi->fileName = tr("..");
    mi->filePath = "DotDot";
    m_frontCovers.insert("DotDot", QImage(m_themePath + "/media/DefaultFolderBack.png"));
    data->videoInfos.append(mi);
    m_data.insert(m_data.count()-1, data);
    endInsertRows();
    m_frontCovers.insert(path, QImage(m_themePath + "/media/DefaultHardDisk.png"));

    startSearchThread();
}

QModelIndex VideoModel::index(int row, int col, const QModelIndex &parent) const
{
    if (col != 0 || row < 0)
        return QModelIndex();
    if (!parent.isValid()) { // top level
        if (row >= m_data.count())
            return QModelIndex();
        return createIndex(row, 0, 0);
    } else { // first level
        Data *data = m_data.value(parent.row());
        if (!data || row >= data->videoInfos.count())
            return QModelIndex();
        return createIndex(row, col, data);
    }
}

QModelIndex VideoModel::parent(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return QModelIndex();
    if (idx.internalPointer() == 0) // top level
        return QModelIndex();
    Data *data = static_cast<Data *>(idx.internalPointer());
    int loc = m_data.indexOf(data);
    if (loc == -1)
        return QModelIndex();
    return createIndex(loc, 0, 0);
}

int VideoModel::columnCount(const QModelIndex &idx) const
{
    Q_UNUSED(idx);
    return 1;
}

int VideoModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_data.count();
    if (parent.internalPointer() == 0) { // top level
        Data *data = m_data.value(parent.row());
        return data ? data->videoInfos.count() : 0;
    } else {
        return 0;
    }
}

QVariant VideoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.internalPointer() == 0) { // top level
        Data *data = m_data.value(index.row());
        if (!data)
            return QVariant();
        if (role == Qt::DisplayRole) {
            return data->name;
        } else if (role == Qt::DecorationRole) {
        } else if (role == DecorationUrlRole) {
            return QUrl("image://qtmediahub/musicmodel" + data->searchPath);
        } else if (role == FilePathRole) {
            return data->searchPath;
        }

        return QVariant();
    }

    Data *data = static_cast<Data *>(index.internalPointer());
    VideoInfo *info = data->videoInfos[index.row()];
    if (role == Qt::DisplayRole) {
        return info->fileName;
    } else if (role == Qt::DecorationRole) {
        return QVariant(); // FIX
//        return decorationPixmap(info);
    } else if (role == FilePathRole) {
        return info->filePath;
    } else if (role == FileNameRole) {
        return info->fileName;
    } else if (role == DecorationUrlRole) {
        return QUrl("image://qtmediahub/musicmodel" + info->filePath);
    } else if (role == LengthRole) {
        return info->length;
    } else if (role == SubtitlesRole) {
        return info->subtitles;
    } else if (role == ChannelsRole) {
        return info->channels;
    } else {
        return QVariant();
    }
}

void VideoModel::addVideo(int row, const VideoInfo &video, const QImage &frontCover)
{
    Data *data = m_data[row];
    beginInsertRows(createIndex(row, 0, 0), data->videoInfos.count(), data->videoInfos.count());
    VideoInfo *mi = new VideoInfo(video);
    if (!frontCover.isNull())
        m_frontCovers.insert(mi->filePath, frontCover);
    else
        m_frontCovers.insert(mi->filePath, QImage(m_themePath + "/media/Fanart_Fallback_Music_Small.jpg"));
    data->videoInfos.append(mi);
    endInsertRows();
}

QPixmap VideoModel::decorationPixmap(const QString &path, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);
    if (!m_frontCovers.contains(path))
        return QPixmap();
    QPixmap pixmap;
    if (!QPixmapCache::find("coverart_" + path, &pixmap)) {
        pixmap = QPixmap::fromImage(m_frontCovers[path]);
        QPixmapCache::insert("coverart_" + path, pixmap);
    }
    *size = pixmap.size();
    return pixmap;
}

QImage VideoModel::decorationImage(const QString &path, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);
    if (!m_frontCovers.contains(path))
        return QImage();
    QImage img = m_frontCovers.value(path);
    *size = img.size();
    return img;
}

void VideoModel::setThemeResourcePath(const QString &themePath)
{
    m_themePath = themePath;
    m_frontCovers.insert("AddNewSource", QImage(m_themePath + "/media/DefaultAddSource.png"));
    reset();
}

VideoModelThread::VideoModelThread(VideoModel *model, int row, const QString &searchPath)
    : m_model(model), m_stop(false), m_row(row), m_searchPath(searchPath)
{
}

VideoModelThread::~VideoModelThread()
{
}

void VideoModelThread::stop()
{
    m_stop = true;
}

void VideoModelThread::run()
{
    emit started();

    Q_ASSERT(!m_searchPath.isEmpty() && m_searchPath != tr("AddNewSource"));

    search();

    emit finished();
}

static bool generateThumbnail(const VideoInfo &info, const QFileInfo &thumbnailInfo)
{
    // cleanup tmp
    QDir dir;
    QFileInfo tmp(thumbnailInfo.absolutePath() + "/00000001.png");
    if (tmp.exists())
        dir.remove(tmp.filePath());

    QString program = "mplayer";
    QStringList arguments;
    arguments << "-ss" << "60";
    arguments << "-nosound";
    arguments << "-vo" << "png";
    arguments << "-frames" << "1";
    arguments << info.filePath;

    qDebug() << "videoModel create thumbnail for:" << info.fileName << thumbnailInfo.absoluteFilePath();

    QProcess *process = new QProcess();
    process->setWorkingDirectory(thumbnailInfo.absolutePath());
    process->start(program, arguments);
    process->waitForFinished();
    delete process;

    if (tmp.exists()) {
        dir.rename(tmp.filePath(), thumbnailInfo.absoluteFilePath());
        return true;
    } else {
        return false;
    }
}

void VideoModelThread::search()
{
    QDirIterator it(m_searchPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (!m_stop && it.hasNext()) {
        VideoInfo info;
        info.filePath = it.next();
        info.fileName = it.fileName();
        QByteArray fileName = QFile::encodeName(info.filePath);

        // TODO actual readout of metadata
        info.length = 0;
        info.channels = 1;
        info.subtitles = 0;

        // Populate music type specific fields
        QImage frontCover;

        QString md5 = QCryptographicHash::hash(QString("file://" + info.filePath).toUtf8(), QCryptographicHash::Md5).toHex();

        // check if thumnail folder exists
        QFileInfo thumbnailFolderInfo(QDir::homePath() + "/.thumbnails/qtmediahub/");
        if (!thumbnailFolderInfo.exists()) {
            QDir dir;
            dir.mkpath(thumbnailFolderInfo.absoluteFilePath());
        }

        QFileInfo thumbnailInfo(thumbnailFolderInfo.absoluteFilePath() + md5 + ".png");
        QFileInfo fileInfo(info.filePath);

        QString ret = "";

        if (thumbnailInfo.exists() || generateThumbnail(info, thumbnailInfo))
            ret = thumbnailInfo.filePath();
        else
            ret = "";

        emit videoFound(m_row, info, frontCover);
   }
}

void VideoModel::dump()
{
    qDebug() << m_data.count() << "elements";
    for (int i = 0; i < m_data.count(); i++) {
        qDebug() << m_data[i]->searchPath;
        for(int j = 0; j < m_data[i]->videoInfos.count(); j++)
            qDebug() << "\t\t" << m_data[i]->videoInfos[j]->filePath;
    }
}

