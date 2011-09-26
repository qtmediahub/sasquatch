#include "abstractmediaplayer.h"

AbstractMediaPlayer::AbstractMediaPlayer(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
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

