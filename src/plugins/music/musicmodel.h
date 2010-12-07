#ifndef MUSICMODEL_H
#define MUSICMODEL_H

#include "../mediamodel.h"

struct MusicInfo : public MediaInfo
{
    MusicInfo() :  MediaInfo(MediaModel::File), year(0), track(0), length(0), bitrate(0), sampleRate(0), channels(0) { }

    // tag info
    QString title;
    QString artist;
    QString album;
    QString comment;
    QString genre;
    quint32 year;
    quint32 track;
    // audio properties
    int  length;
    int  bitrate;
    int  sampleRate;
    int  channels;

    QImage frontCover;
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
        GenreRole
    };

    QVariant data(MediaInfo *mediaInfo, int role) const;
    MediaInfo *readMediaInfo(const QString &filePath); // called from thread
    QImage decoration(MediaInfo *mediaInfo) const;
};

#endif // MUSICMODEL_H

