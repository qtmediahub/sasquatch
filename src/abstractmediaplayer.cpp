#include "abstractmediaplayer.h"

#ifdef SCENEGRAPH
AbstractMediaPlayer::AbstractMediaPlayer(QSGItem *parent)
    : QSGItem(parent),
#else
AbstractMediaPlayer::AbstractMediaPlayer(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
#endif
      m_status(UnknownStatus)
{
}

void AbstractMediaPlayer::setStatus(Status status)
{
    m_status = status;
    emit statusChanged();
}

AbstractMediaPlayer::Status AbstractMediaPlayer::status() const
{
    return m_status;
}

