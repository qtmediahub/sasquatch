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

#include "musicmodel.h"
#include <QDirIterator>
#include <QThreadPool>
#include <QTimer>
#include <QtDebug>

#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/mpeg/mpegfile.h>
#include <taglib/mpeg/id3v2/id3v2tag.h>
#include <taglib/mpeg/id3v2/frames/attachedpictureframe.h>

MusicModel::MusicModel(QObject *parent)
    : QAbstractItemModel(parent),
      m_thread(0)
{
    qRegisterMetaType<MusicInfo>("MusicInfo");

    QHash<int, QByteArray> roleNames;
    roleNames[Qt::DisplayRole] = "display";
    roleNames[DecorationUrlRole] = "decorationUrl";
    roleNames[TitleRole] = "title";
    roleNames[AlbumRole] = "album";
    roleNames[CommentRole] = "comment";
    roleNames[GenreRole] = "genre";
    roleNames[FilePathRole] = "filePath";
    roleNames[FileNameRole] = "fileName";
    setRoleNames(roleNames);

    // FIXME: Move this to the proxy
    addSearchPath(QString() /* meta item */, tr("Add new source"));
}

MusicModel::~MusicModel()
{
    for (int i = 0; i < m_data.count(); i++)
        qDeleteAll(m_data[i]->musicInfos);
    qDeleteAll(m_data);
    delete m_thread;
}

void MusicModel::start()
{
    m_thread = new MusicModelThread(this);
    QThreadPool::globalInstance()->start(m_thread);
    connect(m_thread, SIGNAL(musicFound(int, MusicInfo)), this, SLOT(addMusic(int, MusicInfo)));
}

void MusicModel::stop()
{
    m_thread->stop();
}

void MusicModel::addSearchPath(const QString &path, const QString &name)
{
    beginInsertRows(QModelIndex(), m_data.count()-1, m_data.count()-1);
    m_data.insert(m_data.count()-1, new Data(path, name));
    endInsertRows();
}

QModelIndex MusicModel::index(int row, int col, const QModelIndex &parent) const
{
    if (col != 0 || row < 0)
        return QModelIndex();
    if (!parent.isValid()) { // top level
        if (row >= m_data.count())
            return QModelIndex();
        return createIndex(row, 0, 0);
    } else { // first level
        Data *data = m_data.value(parent.row());
        if (!data || row >= data->musicInfos.count())
            return QModelIndex();
        return createIndex(row, col, data);
    }
}

QModelIndex MusicModel::parent(const QModelIndex &idx) const
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

int MusicModel::columnCount(const QModelIndex &idx) const
{
    Q_UNUSED(idx);
    return 1;
}

int MusicModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_data.count();
    if (parent.internalPointer() == 0) { // top level
        Data *data = m_data.value(parent.row());
        return data ? data->musicInfos.count() : 0;
    } else {
        return 0;
    }
}

QVariant MusicModel::data(const QModelIndex &index, int role) const
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
            return QUrl("image://qtmediahub/musicmodel"); // FIXME: Obviously wong
        } else if (role == FilePathRole) {
            return data->searchPath;
        }

        return QVariant();
    }

    Data *data = static_cast<Data *>(index.internalPointer());
    MusicInfo *info = data->musicInfos[index.row()];
    if (role == Qt::DisplayRole) {
        return info->filePath;
        return info->title + QLatin1String(" (") + info->album + QLatin1String(")");
    } else if (role == Qt::DecorationRole) {
        return QVariant(); // FIX
//        return decorationPixmap(info);
    } else if (role == TitleRole) {
        return info->title;
    } else if (role == AlbumRole) {
        return info->album;
    } else if (role == CommentRole) {
        return info->comment;
    } else if (role == GenreRole) {
        return info->genre;
    } else if (role == FilePathRole) {
        return info->filePath;
    } else if (role == FileNameRole) {
        return info->fileName;
    } else if (role == DecorationUrlRole) {
        return QUrl("image://qtmediahub/musicmodel" + info->filePath);
    } else {
        return QVariant();
    }
}

void MusicModel::addMusic(int row, const MusicInfo &music)
{
    Data *data = m_data[row];
    beginInsertRows(createIndex(row, 0, 0), data->musicInfos.count(), data->musicInfos.count());
    MusicInfo *mi = new MusicInfo(music);
    data->musicInfos.append(mi);
    m_pathToMusicInfo.insert(mi->filePath, mi);
    endInsertRows();
}

QPixmap MusicModel::decorationPixmap(MusicInfo *info) const
{
    QPixmap pixmap;
    if (info->frontCover.isNull()) {
        if (!QPixmapCache::find(m_fanartFallbackKey, &pixmap)) {
            pixmap = QPixmap::fromImage(m_fanartFallbackImage);
            m_fanartFallbackKey = QPixmapCache::insert(pixmap);
        }
    } else if (!QPixmapCache::find(info->frontCoverPixmapKey, &pixmap)) {
        pixmap = QPixmap::fromImage(info->frontCover);
        info->frontCoverPixmapKey = QPixmapCache::insert(pixmap);
    }
    return pixmap;
}

QPixmap MusicModel::decorationPixmap(const QString &path, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);
    if (!m_pathToMusicInfo.contains(path))
        return QPixmap();
    MusicInfo *info = m_pathToMusicInfo[path];
    QPixmap pix = decorationPixmap(info);
    *size = pix.size();
    return pix;
}

QImage MusicModel::decorationImage(const QString &path, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);
    if (!m_pathToMusicInfo.contains(path))
        return QImage();
    MusicInfo *info = m_pathToMusicInfo[path];
    QImage img = info->frontCover;
    if (img.isNull())
        img = m_fanartFallbackImage;
    *size = img.size();
    return img;
}

void MusicModel::setThemeResourcePath(const QString &themePath)
{
    m_themePath = themePath;
    m_fanartFallbackImage = QImage(m_themePath + "/media/Fanart_Fallback_Music_Small.jpg");
    reset();
}

static inline QString fromTagString(const TagLib::String &string)
{
    return QString::fromStdWString(string.toWString());
}

static void popuplateGenericTagInfo(MusicInfo *info, TagLib::Tag *tag)
{
    info->title = fromTagString(tag->title());
    info->artist = fromTagString(tag->artist());
    info->album = fromTagString(tag->album());
    info->comment = fromTagString(tag->comment());
    info->genre = fromTagString(tag->genre());
    info->year = tag->year();
    info->track = tag->track();
}

static void popuplateAudioProperties(MusicInfo *info, TagLib::AudioProperties *properties)
{
    info->length = properties->length();
    info->bitrate = properties->bitrate();
    info->sampleRate = properties->sampleRate();
    info->channels = properties->channels();
}

static void populateFrontCover(MusicInfo *info, TagLib::ID3v2::Tag *id3v2Tag)
{
    TagLib::ID3v2::FrameList frames = id3v2Tag->frameListMap()["APIC"];
    if (frames.isEmpty()) {
        //qDebug() << "No front cover";
        return;
    }

    TagLib::ID3v2::AttachedPictureFrame *selectedFrame = 0;
    if (frames.size() != 1) {
        TagLib::ID3v2::FrameList::Iterator it = frames.begin();
        for (; it != frames.end(); ++it) {
            TagLib::ID3v2::AttachedPictureFrame *frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(*it);
            if (frame && frame->type() != TagLib::ID3v2::AttachedPictureFrame::FrontCover) // BackCover, LeafletPage
                continue;
            selectedFrame = frame;
            break;
        }
    }
    if (!selectedFrame)
        selectedFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(frames.front());
    if (!selectedFrame)
        return;

    QByteArray imageData(selectedFrame->picture().data(), selectedFrame->picture().size());
    QImage attachedImage = QImage::fromData(imageData);
    // ## scale as necessary
    info->frontCover = attachedImage;
}

MusicModelThread::MusicModelThread(MusicModel *model)
    : m_model(model), m_stop(false)
{
}

MusicModelThread::~MusicModelThread()
{
}

void MusicModelThread::stop()
{
    m_stop = true;
}

void MusicModelThread::run()
{
    emit started();

    for (int i = 0; i < m_model->m_data.count(); i++) {
        QString searchPath = m_model->m_data[i]->searchPath;
        if (searchPath.isEmpty())
            continue;
        searchIn(i, m_model->m_data[i]->searchPath);
        if (m_stop)
            break;
    }

    emit finished();
}

void MusicModelThread::searchIn(int row, const QString &searchPath)
{
    QDirIterator it(searchPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (!m_stop && it.hasNext()) {
        MusicInfo info;
        info.filePath = it.next();
        info.fileName = it.fileName();
        QByteArray fileName = QFile::encodeName(info.filePath);
        
        TagLib::FileRef fileRef(fileName.constData());
        if (fileRef.isNull()) {
            // qDebug() << "Dropping " << info.path;
            continue;
        }

        TagLib::File *file = fileRef.file();
        if (TagLib::Tag *tag = file->tag())
            popuplateGenericTagInfo(&info, tag);
        if (TagLib::AudioProperties *audioProperties = file->audioProperties())
            popuplateAudioProperties(&info, audioProperties);

        // Populate music type specific fields
        if (TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File *>(file)) {
            TagLib::ID3v2::Tag *id3v2Tag = mpegFile->ID3v2Tag(false);
            if (id3v2Tag)
                populateFrontCover(&info, id3v2Tag);
        }

        emit musicFound(row, info);
   }

    emit finished();
}

void MusicModel::dump()
{
    qDebug() << m_data.count() << "elements";
    for (int i = 0; i < m_data.count(); i++) {
        qDebug() << m_data[i]->searchPath;
        for(int j = 0; j < m_data[i]->musicInfos.count(); j++)
            qDebug() << "\t\t" << m_data[i]->musicInfos[j]->filePath;
    }
}

