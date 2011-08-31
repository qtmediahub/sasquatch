/****************************************************************************

This file is part of the QtMediaHub project on http://www.qtmediahub.com

Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Girish Ramakrishnan girish@forwardbias.in
Contact:  Nokia Corporation donald.carr@nokia.com
Contact:  Nokia Corporation johannes.zellner@nokia.com

You may use this file under the terms of the BSD license as follows:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#ifndef TESTINGPLAYER_H
#define TESTINGPLAYER_H

#include "abstractmediaplayer.h"

class TestingPlayer : public AbstractMediaPlayer
{
    Q_OBJECT
public:
    explicit TestingPlayer(QObject *parent = 0) :
        AbstractMediaPlayer(parent) { /**/ }

signals:

public slots:
    void setSource(const QString &source) { qDebug() << __FUNCTION__ << source; }
    void setPlaying(bool p) { qDebug() << __FUNCTION__; }
    void setVolume(qreal vol) { qDebug() << __FUNCTION__ << vol; }
    void setPaused(bool p) { qDebug() << __FUNCTION__ << p; }
    void setPlaybackRate(qreal rate) { qDebug() << __FUNCTION__ << rate; }

    void play() { qDebug() << __FUNCTION__; }
    void stop() { qDebug() << __FUNCTION__; }
    void pause() { qDebug() << __FUNCTION__; }
    void resume() { qDebug() << __FUNCTION__; }
    void mute(bool on = true) { qDebug() << __FUNCTION__ << on; }
    void setPosition(int position) { qDebug() << __FUNCTION__ << position; }
    void setPositionPercent(qreal position) { qDebug() << __FUNCTION__ << position; }
    void setVolume(int volume) { qDebug() << __FUNCTION__ << volume; }
    void setVolumePercent(qreal volume) { qDebug() << __FUNCTION__ << volume; }
};

#endif // TESTINGPLAYER_H
