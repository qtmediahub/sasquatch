#include "mediaplayerhelper.h"

#include <QtDebug>

MediaPlayerHelper::MediaPlayerHelper(QObject *parent) :
    QObject(parent),
    m_mediaInfo(0)
{
    setObjectName("qmhmediaplayer");
}

void MediaPlayerHelper::playRemoteSource(QString uri, int position)
{
    qDebug() << "play remote source" << uri << position;

    MediaInfo *info = new MediaInfo(MediaModel::File, uri, MediaModel::Video);

    qDebug() << "info" << info << info->filePath;

    m_mediaInfo = info;
    m_position = position;

    emit mediaInfoChanged();
    emit playRemoteSourceRequested(info, m_position);
}
