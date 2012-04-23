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

#include "xineplayer.h"

#include <xine.h>
#include <xine/xineutils.h>

class XinePlayer::Private
{
public:
    Private();
    ~Private();

    QString mediaPath;
    bool playing;

    xine_t              *xine;
    xine_stream_t       *stream;
    xine_audio_port_t   *ao_port;
    xine_event_queue_t  *event_queue;
};

XinePlayer::Private::Private()
    : playing(false)
{
    char configfile[2048];

    xine = xine_new();
    sprintf(configfile, "%s%s", xine_get_homedir(), "/.xine/config");
    xine_config_load(xine, configfile);
    xine_init(xine);

    ao_port = xine_open_audio_driver(xine , "auto" , NULL);
    stream = xine_stream_new(xine, ao_port, NULL);
    event_queue = xine_event_new_queue(stream);
}

XinePlayer::Private::~Private()
{
    xine_close(stream);
    xine_event_dispose_queue(event_queue);
    xine_dispose(stream);
    if (ao_port)
        xine_close_audio_driver(xine, ao_port);
    xine_exit(xine);

    //delete xine;
    //delete stream;
    //delete ao_port;
    //delete event_queue;
}

XinePlayer::XinePlayer(QObject *parent) :
    AbstractMediaPlayer(),
    d(new Private())
{
    setParent(parent);
}

XinePlayer::~XinePlayer()
{
    delete d;
    d = 0;
}

void XinePlayer::setSource(const QString &source)
{
    //Strip the formatting off the incoming uri
    //file:// /
    d->mediaPath = source.mid(7);
}

void XinePlayer::stop() {
    d->playing = false;
    xine_close(d->stream);
    emit playingChanged();
}

void XinePlayer::pause() {
    xine_set_param(d->stream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE);
}

void XinePlayer::resume() {
    xine_set_param(d->stream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL);
}

void XinePlayer::play()
{
    d->playing = true;
    if ((!xine_open(d->stream, d->mediaPath.toAscii().constData()))
            || (!xine_play(d->stream, 0, 0))) {
        printf("Unable to open mrl '%s'\n", d->mediaPath.toAscii().constData());
    }
    emit playingChanged();
}

void XinePlayer::mute(bool on)
{
    if (on) {
        xine_set_param(d->stream, XINE_PARAM_AUDIO_AMP_MUTE, 1);
    } else {
        xine_set_param(d->stream, XINE_PARAM_AUDIO_AMP_MUTE, 0);
    }
}

void XinePlayer::setPosition(int position) { Q_UNUSED(position); }
void XinePlayer::setPositionPercent(qreal position) { Q_UNUSED(position); }

void XinePlayer::setVolumePercent(qreal volume) {
    //Wants an int for volume
    //Attempting to map to Video Item levels on local machine
    if (volume > 0.20) {
        qDebug() << "Exhibiting cowardise, capping volume";
        volume = 0.20;
    }
    xine_set_param(d->stream, XINE_PARAM_AUDIO_VOLUME, volume*100);
}
