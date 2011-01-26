#ifndef MUSICMODEL_H
#define MUSICMODEL_H

#include "../mediamodel.h"
#include "../mediainfo.h"

class MusicInfo : public MediaInfo
{
public:
    MusicInfo(const QString &path) :  MediaInfo(MediaModel::File, path, MediaModel::Music) { }

    // tag info
    QMap<QString, QVariant> tagProperties;
};

class MusicModel : public MediaModel
{
    Q_OBJECT
public:
    MusicModel(QObject *parent = 0);
    ~MusicModel();

    enum CustomRoles {
        TitleRole = Qt::UserRole + 100,
        ArtistRole,
        AlbumRole,
        CommentRole,
        GenreRole,
        TagPropertiesRole
    };

    QVariant data(MediaInfo *mediaInfo, int role) const;
    MediaInfo *readMediaInfo(const QString &filePath); // called from thread
};

#endif // MUSICMODEL_H

