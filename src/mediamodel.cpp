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

#include "mediamodel.h"
#include <QDirIterator>
#include <QTimer>
#include <QtDebug>

#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/mpeg/mpegfile.h>
#include <taglib/mpeg/id3v2/id3v2tag.h>
#include <taglib/mpeg/id3v2/frames/attachedpictureframe.h>

MediaModel::MediaModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_mediaPath = QDir::currentPath();
    QTimer::singleShot(0, this, SLOT(update()));
}

MediaModel::MediaModel(const QString &mediaPath, QObject *parent)
    : QAbstractItemModel(parent),
      m_mediaPath(mediaPath)
{
    QTimer::singleShot(0, this, SLOT(update()));
}

MediaModel::~MediaModel()
{
}

void MediaModel::setMediaPath(const QString &newPath)
{
    if (newPath == m_mediaPath)
        return;
    m_mediaPath = newPath;
    update();
    emit mediaPathChanged();
}

QString MediaModel::mediaPath() const
{
    return m_mediaPath;
}

QModelIndex MediaModel::index(int row, int col, const QModelIndex &parent) const
{
    if (parent.isValid() || row < 0 || row >= m_mediaInfos.count() || col != 0)
        return QModelIndex();
    return createIndex(row, 0, 0);
}

QModelIndex MediaModel::parent(const QModelIndex &idx) const
{
    Q_UNUSED(idx);
    return QModelIndex();
}

int MediaModel::columnCount(const QModelIndex &idx) const
{
    return idx.isValid() ? 0 : 1;
}

int MediaModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_mediaInfos.count();
}

QVariant MediaModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        return m_mediaInfos[index.row()].path;
    } else if (role == Qt::DecorationRole) {
        return m_mediaInfos[index.row()].frontCover;
    } else {
        return QVariant();
    }
}

static inline QString fromTagString(const TagLib::String &string)
{
    return QString::fromStdWString(string.toWString());
}

static void popuplateGenericTagInfo(MediaInfo *info, TagLib::Tag *tag)
{
    info->title = fromTagString(tag->title());
    info->artist = fromTagString(tag->artist());
    info->album = fromTagString(tag->album());
    info->comment = fromTagString(tag->comment());
    info->genre = fromTagString(tag->genre());
    info->year = tag->year();
    info->track = tag->track();
}

static void popuplateAudioProperties(MediaInfo *info, TagLib::AudioProperties *properties)
{
    info->length = properties->length();
    info->bitrate = properties->bitrate();
    info->sampleRate = properties->sampleRate();
    info->channels = properties->channels();
}

static void populateFrontCover(MediaInfo *info, TagLib::ID3v2::Tag *id3v2Tag)
{
    TagLib::ID3v2::FrameList frames = id3v2Tag->frameListMap()["APIC"];
    if (frames.isEmpty()) {
        qDebug() << "No front cover";
        return;
    }

    TagLib::ID3v2::AttachedPictureFrame *selectedFrame = 0;
    if (frames.size() != 1) {
        TagLib::ID3v2::FrameList::Iterator it = frames.begin();
        for (; it != frames.end(); ++it) {
            TagLib::ID3v2::AttachedPictureFrame *frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(*it);
            if (frame && frame->type() != TagLib::ID3v2::AttachedPictureFrame::FrontCover)
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
    info->frontCover = QPixmap::fromImage(attachedImage);
}

void MediaModel::update()
{
    beginResetModel();
    m_mediaInfos.clear();
    QDirIterator it(m_mediaPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        MediaInfo info;
        info.path = it.next();
        QByteArray fileName = QFile::encodeName(info.path);
        
        if (fileName.endsWith(".mp3")) {
            TagLib::MPEG::File mpegFile(fileName.constData(), true, TagLib::AudioProperties::Accurate);
            TagLib::ID3v2::Tag *id3v2Tag = mpegFile.ID3v2Tag(false);
            if (id3v2Tag)
                populateFrontCover(&info, id3v2Tag);
            if (TagLib::Tag *tag = mpegFile.tag())
                popuplateGenericTagInfo(&info, mpegFile.tag());
            if (TagLib::AudioProperties *properties = mpegFile.audioProperties())
                popuplateAudioProperties(&info, mpegFile.audioProperties());
        } else {
            TagLib::FileRef f(fileName.constData());
            if (f.isNull()) {
                // qDebug() << "Dropping " << info.path;
                continue;
            }
            if (TagLib::Tag *tag = f.tag())
                popuplateGenericTagInfo(&info, tag);
            if (TagLib::AudioProperties *properties = f.audioProperties())
                popuplateAudioProperties(&info, properties);
        }

        m_mediaInfos.append(info);
    }
    endResetModel();
}

