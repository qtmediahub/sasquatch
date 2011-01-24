#include "musicmodel.h"

#include <QFile>
#include <QFileInfo>

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
    roleNames[TagPropertiesRole] = "tagProperties";
    setRoleNames(roleNames);
}

MusicModel::~MusicModel()
{
}

QVariant MusicModel::data(MediaInfo *mediaInfo, int role) const
{
    MusicInfo *info = static_cast<MusicInfo *>(mediaInfo);

    if (role == Qt::DisplayRole) {
        if (info->tagProperties["title"].toString().isEmpty())
            return info->name;
        QString title = info->tagProperties["title"].toString();
        if (!info->tagProperties["album"].toString().isEmpty())
            title.append(QLatin1String(" (") + info->tagProperties["album"].toString() + QLatin1String(")"));
        return title;
    } else if (role == TitleRole) {
        return info->tagProperties["title"];
    } else if (role == AlbumRole) {
        return info->tagProperties["album"];
    } else if (role == CommentRole) {
        return info->tagProperties["comment"];
    } else if (role == GenreRole) {
        return info->tagProperties["genre"];
    } else if (role == TagPropertiesRole) {
        return info->tagProperties;
    } else {
        return QVariant();
    }
}

static inline QString fromTagString(const TagLib::String &string)
{
    return QString::fromStdWString(string.toWString());
}

static void popuplateGenericTagInfo(MusicInfo *info, TagLib::Tag *tag)
{
    info->tagProperties["title"] = fromTagString(tag->title());
    info->tagProperties["artist"] = fromTagString(tag->artist());
    info->tagProperties["album"] = fromTagString(tag->album());
    info->tagProperties["comment"] = fromTagString(tag->comment());
    info->tagProperties["genre"] = fromTagString(tag->genre());
    info->tagProperties["year"] = tag->year();
    info->tagProperties["track"] = tag->track();
}

static void popuplateAudioProperties(MusicInfo *info, TagLib::AudioProperties *properties)
{
    info->tagProperties["length"] = properties->length();
    info->tagProperties["bitrate"] = properties->bitrate();
    info->tagProperties["sampleRate"] = properties->sampleRate();
    info->tagProperties["channels"] = properties->channels();
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

    MusicInfo *info = new MusicInfo(filePath);

    TagLib::File *file = fileRef.file();
    if (TagLib::Tag *tag = file->tag())
        popuplateGenericTagInfo(info, tag);
    if (TagLib::AudioProperties *audioProperties = file->audioProperties())
        popuplateAudioProperties(info, audioProperties);

    // check if we already have a local cover art for this file
    QFileInfo thumbnailInfo(info->thumbnailPath);

    if (!thumbnailInfo.exists()) {
        if (TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File *>(file)) {
            TagLib::ID3v2::Tag *id3v2Tag = mpegFile->ID3v2Tag(false);
            if (id3v2Tag) {
                QImage tmp = readFrontCover(id3v2Tag);
                if (!tmp.isNull()) {
                    tmp = tmp.width() <= previewWidth() ? tmp : tmp.scaledToWidth(previewWidth(), Qt::SmoothTransformation);
                    tmp.save(thumbnailInfo.filePath());
                } else {
                    info->thumbnailPath = themeResourcePath() + "/media/DefaultAudio.png";
                }
            }
        }
    }

    return info;
}

