#include "musicmodel.h"

#include <QFile>

#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/mpeg/mpegfile.h>
#include <taglib/mpeg/id3v2/id3v2tag.h>
#include <taglib/mpeg/id3v2/frames/attachedpictureframe.h>

MusicModel::MusicModel(QObject *parent)
    : MediaModel(MediaModel::Music, parent)
{
    QHash<int, QByteArray> roleNames = MediaModel::roleNames();
    roleNames[TitleRole] = "title";
    roleNames[AlbumRole] = "album";
    roleNames[CommentRole] = "comment";
    roleNames[GenreRole] = "genre";
    setRoleNames(roleNames);
}

MusicModel::~MusicModel()
{
}

QVariant MusicModel::data(MediaInfo *mediaInfo, int role) const
{
    MusicInfo *info = static_cast<MusicInfo *>(mediaInfo);

    if (role == Qt::DisplayRole) {
        if (info->title.isEmpty())
            return info->name;
        QString title = info->title;
        if (!info->album.isEmpty())
            title.append(QLatin1String(" (") + info->album + QLatin1String(")"));
        return title;
    } else if (role == TitleRole) {
        return info->title;
    } else if (role == AlbumRole) {
        return info->album;
    } else if (role == CommentRole) {
        return info->comment;
    } else if (role == GenreRole) {
        return info->genre;
    } else {
        return QVariant();
    }
}

QImage MusicModel::decoration(MediaInfo *info) const
{
    QImage frontCover = (static_cast<MusicInfo *>(info))->frontCover;
    if (frontCover.isNull())
        return QImage(themeResourcePath() + "/media/Fanart_Fallback_Music_Small.jpg"); // FIXME: Make this configurable
    return frontCover;
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

static QImage readFrontCover(TagLib::ID3v2::Tag *id3v2Tag)
{
    TagLib::ID3v2::FrameList frames = id3v2Tag->frameListMap()["APIC"];
    if (frames.isEmpty()) {
        //qDebug() << "No front cover";
        return QImage();
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
        return QImage();

    QByteArray imageData(selectedFrame->picture().data(), selectedFrame->picture().size());
    QImage attachedImage = QImage::fromData(imageData);
    // ## scale as necessary
    return attachedImage;
}

MediaInfo *MusicModel::readMediaInfo(const QString &filePath)
{
    QByteArray fileName = QFile::encodeName(filePath);
    
    TagLib::FileRef fileRef(fileName.constData());
    if (fileRef.isNull()) {
        // qDebug() << "Dropping " << info.path;
        return 0;
    }

    MusicInfo *info = new MusicInfo;

    TagLib::File *file = fileRef.file();
    if (TagLib::Tag *tag = file->tag())
        popuplateGenericTagInfo(info, tag);
    if (TagLib::AudioProperties *audioProperties = file->audioProperties())
        popuplateAudioProperties(info, audioProperties);

    if (TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File *>(file)) {
        TagLib::ID3v2::Tag *id3v2Tag = mpegFile->ID3v2Tag(false);
        if (id3v2Tag)
            info->frontCover = readFrontCover(id3v2Tag);
    }

    return info;
}

