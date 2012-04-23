/** This file is part of Qt Media Hub**

Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation qmh-development@qt-project.org

You may use this file under the terms of the BSD license
as follows:

Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of Nokia Corporation and its Subsidiary(-ies)
nor the names of its contributors may be used to endorse or promote
products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. **/

#ifndef ABSTRACTMEDIAPLAYER_H
#define ABSTRACTMEDIAPLAYER_H

#ifdef QT5
#include <QQuickItem>
#else
#include <QDeclarativeItem>
#endif

#include <QDebug>

#include "global.h"

// Media Player API used by the "non-mobility" QML Video element
#ifdef QT5
class QMH_EXPORT AbstractMediaPlayer : public QQuickItem
#else
class QMH_EXPORT AbstractMediaPlayer : public QDeclarativeItem
#endif
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool hasVideo READ hasVideo NOTIFY hasVideoChanged)
    Q_PROPERTY(bool hasAudio READ hasAudio NOTIFY hasAudioChanged)

    Q_PROPERTY(bool playing READ playing WRITE setPlaying NOTIFY playingChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(int position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(bool seekable READ seekable NOTIFY seekableChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(qreal playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY playbackRateChanged)
    Q_PROPERTY(int duration READ duration NOTIFY durationChanged)

public:
    enum Status {
        UnknownStatus,
        NoMedia,
        Loading,
        Loaded,
        Stalled,
        Buffering,
        Buffered,
        EndOfMedia,
        InvalidMedia
    };
    Q_ENUMS(Status)

#ifdef QT5
    explicit AbstractMediaPlayer(QQuickItem *parent = 0);
#else
    explicit AbstractMediaPlayer(QDeclarativeItem *parent = 0);
#endif
    virtual ~AbstractMediaPlayer() { /**/ }

    virtual QString source() const { return QString(); }
    virtual Status status() const;

    virtual bool hasVideo() const { return false; }
    virtual bool hasAudio() const { return false; }

    virtual bool playing() const { return false; }
    virtual qreal volume() const { return 0; }
    virtual int position() const { return 0; }
    virtual bool seekable() const { return false; }
    virtual bool paused() const { return false; }
    virtual qreal playbackRate() const { return 1; }
    virtual int duration() const { return 0; }

signals:
    void sourceChanged();
    void statusChanged();
    void hasAudioChanged();
    void hasVideoChanged();

    void playingChanged();
    void volumeChanged();
    void positionChanged();
    void seekableChanged();
    void pausedChanged();
    void playbackRateChanged();
    void durationChanged();

public slots:
    virtual void setSource(const QString &source) { Q_UNUSED(source); }
    virtual void setPlaying(bool p) { p ? play() : stop(); }
    virtual void setVolume(qreal vol) { Q_UNUSED(vol); }
    virtual void setPaused(bool p) { p ? pause() : resume(); }
    virtual void setPlaybackRate(qreal rate) { Q_UNUSED(rate); }

    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void mute(bool on = true) = 0;
    virtual void setPosition(int position) = 0;
    virtual void setPositionPercent(qreal position) = 0;
    virtual void setVolumePercent(qreal volume) = 0;

protected:
    virtual void setStatus(Status status);

private:
    Status m_status;
};

#endif // ABSTRACTMEDIAPLAYER_H

