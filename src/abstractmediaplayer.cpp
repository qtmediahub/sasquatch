#include "abstractmediaplayer.h"

AbstractMediaPlayer::AbstractMediaPlayer(QObject *parent)
    : QObject(parent),
      m_status(UnknownStatus)
{
}

void AbstractMediaPlayer::setSource(const QString &source)
{
    m_source = source;
    emit sourceChanged();
}

QString AbstractMediaPlayer::source() const
{
    return m_source;
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

