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

MusicModel::MusicModel(const QString &musicPath, QObject *parent)
    : QAbstractItemModel(parent),
      m_musicPath(musicPath)
{
    qRegisterMetaType<MusicInfo>("MusicInfo");

    QHash<int, QByteArray> roleNames;
    roleNames[Qt::DisplayRole] = "display";
    roleNames[TitleRole] = "title";
    roleNames[AlbumRole] = "album";
    roleNames[CommentRole] = "comment";
    roleNames[GenreRole] = "genre";
    roleNames[FilePathRole] = "filePath";
    roleNames[FileNameRole] = "fileName";
    setRoleNames(roleNames);

    m_thread = new MusicModelThread(this);
    QThreadPool::globalInstance()->start(m_thread);
    connect(m_thread, SIGNAL(musicFound(MusicInfo)), this, SLOT(addMusic(MusicInfo)));
}

MusicModel::~MusicModel()
{
    delete m_thread;
}

QString MusicModel::musicPath() const
{
    return m_musicPath;
}

QModelIndex MusicModel::index(int row, int col, const QModelIndex &parent) const
{
    if (parent.isValid() || row < 0 || row >= m_musicInfos.count() || col != 0)
        return QModelIndex();
    return createIndex(row, 0, 0);
}

QModelIndex MusicModel::parent(const QModelIndex &idx) const
{
    Q_UNUSED(idx);
    return QModelIndex();
}

int MusicModel::columnCount(const QModelIndex &idx) const
{
    return idx.isValid() ? 0 : 1;
}

int MusicModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_musicInfos.count();
}

QVariant MusicModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const MusicInfo &info = m_musicInfos[index.row()];
    if (role == Qt::DisplayRole) {
        return info.title + QLatin1String(" (") + info.album + QLatin1String(")");
    } else if (role == Qt::DecorationRole) {
        QPixmap pixmap;
        if (!QPixmapCache::find(info.frontCoverPixmapKey, &pixmap)) {
            pixmap = QPixmap::fromImage(info.frontCover);
            info.frontCoverPixmapKey = QPixmapCache::insert(pixmap);
        }
        return pixmap;
    } else if (role == TitleRole) {
        return info.title;
    } else if (role == AlbumRole) {
        return info.album;
    } else if (role == CommentRole) {
        return info.comment;
    } else if (role == GenreRole) {
        return info.genre;
    } else if (role == FilePathRole) {
        return info.filePath;
    } else if (role == FileNameRole) {
        return info.fileName;
    } else {
        return QVariant();
    }
}

void MusicModel::addMusic(const MusicInfo &music)
{
    beginInsertRows(QModelIndex(), m_musicInfos.count(), m_musicInfos.count());
    m_musicInfos.append(music);
    endInsertRows();
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
    : m_model(model)
{
}

MusicModelThread::~MusicModelThread()
{
}

void MusicModelThread::run()
{
    emit started();
    QDirIterator it(m_model->musicPath(), QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
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

        emit musicFound(info);
   }

    emit finished();
}

