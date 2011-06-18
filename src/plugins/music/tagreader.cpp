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

