/****************************************************************************

This file is part of the QtMediaHub project on http://www.qtmediahub.com

Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Girish Ramakrishnan girish@forwardbias.in
Contact:  Donald Carr sirspudd@gmail.com
Contact:  Nokia Corporation johannes.zellner@nokia.com

You may use this file under the terms of the BSD license as follows:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include "mediaplayervlc.h"

#include <QWidget>
#include <QApplication>

#ifdef QT5
MediaPlayerVLC::MediaPlayerVLC(QQuickItem *parent) :
#else
MediaPlayerVLC::MediaPlayerVLC(QDeclarativeItem *parent) :
#endif
    AbstractMediaPlayer(parent),
    m_source(""),
    m_hasVideo(false),
    m_hasAudio(false),
    m_playing(false),
    m_volume(0),
    m_position(0),
    m_seekable(false),
    m_paused(false),
    m_playbackRate(0),
    m_duration(0)
{
    /* Load the VLC engine */
    m_instance = libvlc_new(0, NULL);

    m_surface = new QWidget();
    QPalette p = m_surface->palette();
    p.setColor(QPalette::Background, QColor(Qt::black));
    m_surface->setPalette(p);
    m_surface->show();

    startTimer(100);
}

MediaPlayerVLC::~MediaPlayerVLC()
{
    if (m_media) {
        libvlc_media_release(m_media);
        m_media = 0;
    }

    if (m_mediaPlayer) {
        libvlc_media_player_release(m_mediaPlayer);
        m_mediaPlayer = 0;
    }

    if (m_instance) {
        libvlc_release(m_instance);
        m_instance = 0;
    }

    if (m_surface) {
        delete m_surface;
        m_surface = 0;
    }
}

void MediaPlayerVLC::timerEvent(QTimerEvent *)
{
    if (!m_media || !m_mediaPlayer)
        return;

    libvlc_media_track_info_t *tracks;
    int count = libvlc_media_get_tracks_info(m_media, &tracks);
    if (count >= 1) {
        setHasAudio((tracks->i_type == libvlc_track_audio || tracks->i_type == libvlc_track_video));
        setHasVideo(tracks->i_type == libvlc_track_video);
    }
    delete tracks;

    int v = libvlc_audio_get_volume(m_mediaPlayer);
    if (m_volume != v) {
        m_volume = v;
        emit volumeChanged();
    }

    float p = libvlc_media_player_get_position(m_mediaPlayer);
    if (m_position != p) {
        m_position = p;
        emit positionChanged();
    }

    libvlc_state_t state = libvlc_media_player_get_state(m_mediaPlayer);

    if (state == libvlc_Opening)
        setStatus(Loading);
    else if (state == libvlc_Buffering)
        setStatus(Buffering);
    else if (state == libvlc_Playing || state == libvlc_Paused || state == libvlc_Stopped)
        setStatus(Loaded);
    else if (state == libvlc_Ended)
        setStatus(EndOfMedia);
    else if (state == libvlc_Error)
        setStatus(InvalidMedia);
    else
        setStatus(UnknownStatus);

    if (state == libvlc_Paused) {
        if (!m_paused) {
            m_paused = true;
            emit pausedChanged();
        }
    }

    if (state == libvlc_Playing) {
        if (!m_playing) {
            m_playing = true;
            emit playingChanged();
        }
    }

    if (state == libvlc_Stopped) {
        if (m_playing) {
            m_playing = false;
            emit playingChanged();
        }
    }

    bool s = libvlc_media_player_is_seekable(m_mediaPlayer);
    if (m_seekable != s) {
        m_seekable = s;
        emit seekableChanged();
    }

    float r = libvlc_media_player_get_rate(m_mediaPlayer);
    if (m_playbackRate != r) {
        m_playbackRate = r;
        emit playbackRateChanged();
    }

    libvlc_time_t time = libvlc_media_player_get_length(m_mediaPlayer);
    if (m_duration != time) {
        m_duration = time;
        emit durationChanged();
    }
}


// setter
void MediaPlayerVLC::setSource(const QString &source)
{
    if (m_source == source)
        return;

    m_source = source;
    emit sourceChanged();

    if (m_media) {
        /* No need to keep the media now */
        libvlc_media_release(m_media);
        m_media = 0;
    }

    /* Create a new item */
    m_media = libvlc_media_new_path (m_instance, source.toLatin1());

    /* Create a media player playing environement */
    m_mediaPlayer = libvlc_media_player_new_from_media(m_media);
    libvlc_media_player_set_xwindow(m_mediaPlayer, (uint)m_surface->effectiveWinId());

    emit hasVideoChanged();
}

void MediaPlayerVLC::play()
{
    if (!m_mediaPlayer)
        return;

    /* play the media_player */
    libvlc_media_player_play(m_mediaPlayer);
}

void MediaPlayerVLC::stop()
{
    if (!m_mediaPlayer)
        return;

    /* Stop playing */
    libvlc_media_player_stop(m_mediaPlayer);
}

void MediaPlayerVLC::pause()
{
    if (!m_mediaPlayer)
        return;

    libvlc_media_player_set_pause(m_mediaPlayer, true);
}

void MediaPlayerVLC::resume()
{
    if (!m_mediaPlayer)
        return;

    libvlc_media_player_set_pause(m_mediaPlayer, false);
}

void MediaPlayerVLC::mute(bool on)
{
    if (!m_mediaPlayer)
        return;

    libvlc_audio_set_mute(m_mediaPlayer, on);
}

void MediaPlayerVLC::setPosition(int position)
{
    if (!m_mediaPlayer)
        return;

    if (m_duration == 0)
        return;

    libvlc_media_player_set_position(m_mediaPlayer, (qreal)position/m_duration);
}

void MediaPlayerVLC::setPositionPercent(qreal position)
{
    if (!m_mediaPlayer)
        return;

    libvlc_media_player_set_position(m_mediaPlayer, position);
}

void MediaPlayerVLC::setVolumePercent(qreal volume)
{
    if (!m_mediaPlayer)
        return;

    libvlc_audio_set_volume(m_mediaPlayer, volume*100.0);
}

void MediaPlayerVLC::setVolume(qreal volume)
{
    if (!m_mediaPlayer)
        return;

    libvlc_audio_set_volume(m_mediaPlayer, volume*100.0);
}

void MediaPlayerVLC::setPlaybackRate(qreal rate)
{
    if (!m_mediaPlayer)
        return;

    libvlc_media_player_set_rate(m_mediaPlayer, rate);
}

// getter
QString MediaPlayerVLC::source() const
{
    return m_source;
}

bool MediaPlayerVLC::hasVideo() const
{
    return m_hasVideo;
}

bool MediaPlayerVLC::hasAudio() const
{
    return m_hasAudio;
}

bool MediaPlayerVLC::playing() const
{
    return m_playing;
}

qreal MediaPlayerVLC::volume() const
{
    return m_volume/100.0;
}

int MediaPlayerVLC::position() const
{
    return m_position*m_duration;
}

bool MediaPlayerVLC::seekable() const
{
    return m_seekable;
}

bool MediaPlayerVLC::paused() const
{
    return m_paused;
}

qreal MediaPlayerVLC::playbackRate() const
{
    return m_playbackRate;
}

int MediaPlayerVLC::duration() const
{
    return m_duration;
}


// private setter
void MediaPlayerVLC::setHasAudio(bool a)
{
    if (a == m_hasAudio)
        return;

    m_hasAudio = a;
    emit hasAudioChanged();
}

void MediaPlayerVLC::setHasVideo(bool v)
{
    if (v == m_hasVideo)
        return;

    m_hasVideo = v;
    emit hasVideoChanged();
}

void MediaPlayerVLC::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    m_surface->setGeometry(newGeometry.toRect());
}


