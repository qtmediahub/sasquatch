#ifndef MUSICMODEL_H
#define MUSICMODEL_H

#include "../mediamodel.h"

struct MusicInfo : public MediaInfo
{
    MusicInfo() :  MediaInfo(MediaModel::File) { }

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

