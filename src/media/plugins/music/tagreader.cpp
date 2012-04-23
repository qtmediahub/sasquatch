/** This file is part of Qt Media Hub**

Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation qmh-development@qt-project.org

You may use this file under the terms of the BSD license
as follows:

Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of Nokia Corporation and its Subsidiary(-ies)
nor the names of its contributors may be used to endorse or promote
products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. **/

#include "tagreader.h"
#include <QFile>

TagReader::TagReader(const QString &file)
    : m_tag(0), m_audioProperties(0)
{
    QByteArray fileName = QFile::encodeName(file);

    m_fileRef = TagLib::FileRef (fileName.constData());
    if (!m_fileRef.isNull()) {
        TagLib::File *file = m_fileRef.file();
        m_tag = file->tag();
        m_audioProperties = file->audioProperties();
    }
}

TagReader::~TagReader()
{
}

bool TagReader::isValid() const
{
    return m_tag != 0;
}

bool TagReader::hasAudioProperties() const
{
    return m_audioProperties != 0;
}

static inline QString fromTagString(const TagLib::String &string)
{
    //We require STL support for this method to exist!
    return QString::fromStdWString(string.toWString());
}

QString TagReader::title() const
{
    return m_tag ? fromTagString(m_tag->title()) : QString();
}

QString TagReader::album() const
{
    return m_tag ? fromTagString(m_tag->album()) : QString();
}

QString TagReader::artist() const
{
    return m_tag ? fromTagString(m_tag->artist()) : QString();
}

int TagReader::track() const
{
    return m_tag ? m_tag->track() : -1;
}

int TagReader::year() const
{
    return m_tag ? m_tag->year() : -1;
}

QString TagReader::genre() const
{
    return m_tag ? fromTagString(m_tag->genre()) : QString();
}

QString TagReader::comment() const
{
    return m_tag ? fromTagString(m_tag->comment()) : QString();
}

int TagReader::length() const
{
    return m_audioProperties ? m_audioProperties->length() : -1;
}

int TagReader::bitrate() const
{
    return m_audioProperties ? m_audioProperties->bitrate() : -1;
}

int TagReader::sampleRate() const
{
    return m_audioProperties ? m_audioProperties->sampleRate() : -1;
}

int TagReader::channels() const
{
    return m_audioProperties ? m_audioProperties->channels() : -1;
}

static QByteArray readFrontCover(TagLib::ID3v2::Tag *id3v2Tag)
{
    TagLib::ID3v2::FrameList frames = id3v2Tag->frameListMap()["APIC"];
    if (frames.isEmpty()) {
        //qDebug() << "No front cover";
        return QByteArray();
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
        return QByteArray();

    QByteArray imageData(selectedFrame->picture().data(), selectedFrame->picture().size());
    return imageData;
}

QByteArray TagReader::thumbnail() const
{
    TagLib::File *file = m_fileRef.file();
    TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File *>(file);
    if (!mpegFile)
        return QByteArray();

    TagLib::ID3v2::Tag *id3v2Tag = mpegFile->ID3v2Tag(false);
    if (!id3v2Tag)
        return QByteArray();

    return readFrontCover(id3v2Tag);
}

QImage TagReader::thumbnailImage() const
{
    QByteArray data = thumbnail();
    if (data.isNull())
        return QImage();
    QImage attachedImage = QImage::fromData(data);
    // ## scale as necessary
    return attachedImage;
}

