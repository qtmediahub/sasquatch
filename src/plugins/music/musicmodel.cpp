#include "musicmodel.h"

#include <QFile>
#include <QFileInfo>

#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/mpeg/mpegfile.h>
#include <taglib/mpeg/id3v2/id3v2tag.h>
#include <taglib/mpeg/id3v2/frames/attachedpictureframe.h>

#include "tagreader.h"

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

MediaInfo *MusicModel::readMediaInfo(const QString &filePath)
{
    TagReader reader(filePath);

    if (!reader.isValid())
        return 0;

    MusicInfo *info = new MusicInfo(filePath);

    info->tagProperties["title"] = reader.title();
    info->tagProperties["artist"] = reader.artist();
    info->tagProperties["album"] = reader.album();
    info->tagProperties["comment"] = reader.comment();
    info->tagProperties["genre"] = reader.genre();
    info->tagProperties["year"] = reader.year();
    info->tagProperties["track"] = reader.track();

    if (reader.hasAudioProperties()) {
        info->tagProperties["length"] = reader.length();
        info->tagProperties["bitrate"] = reader.bitrate();
        info->tagProperties["sampleRate"] = reader.sampleRate();
        info->tagProperties["channels"] = reader.channels();
    }

    // check if we already have a local cover art for this file
    QFileInfo thumbnailInfo(info->thumbnailPath);

    if (!thumbnailInfo.exists()) {
        QImage tmp = reader.thumbnailImage();
        if (!tmp.isNull()) {
            tmp = tmp.width() <= previewWidth() ? tmp : tmp.scaledToWidth(previewWidth(), Qt::SmoothTransformation);
            tmp.save(thumbnailInfo.filePath());
            return info;
        }
    }

    // TODO: file names should no be hardcoded
    info->thumbnailPath = themeResourcePath() + "DefaultAudio.png";
    return info;
}

