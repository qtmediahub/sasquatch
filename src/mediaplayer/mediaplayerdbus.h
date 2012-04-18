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

#ifndef MEDIAPLAYER_DBUS_H
#define MEDIAPLAYER_DBUS_H

#include "abstractmediaplayer.h"

#include <QtDBus>

class MediaPlayerDbus : public AbstractMediaPlayer
{
    Q_OBJECT
public:
#ifdef QT5
    explicit MediaPlayerDbus(QQuickItem *parent = 0)
#else
    explicit MediaPlayerDbus(QDeclarativeItem *parent = 0)
#endif
    : AbstractMediaPlayer(parent)
     , interface(new QDBusInterface(QMH_PLAYER_DBUS_SERVICENAME, "/", QString(), QDBusConnection::sessionBus(), this))
{
}

signals:

public slots:
    void setSource(const QString &source) { AbstractMediaPlayer::setSource(source); interface->call(__FUNCTION__, source); }
    void play() { interface->call(__FUNCTION__); }
    void stop() { interface->call(__FUNCTION__); }
    void pause() { interface->call(__FUNCTION__); }
    void resume() { interface->call(__FUNCTION__); }
    void mute(bool on = true) { interface->call(__FUNCTION__, on); }
    void setPosition(int position) { interface->call(__FUNCTION__, position); }
    void setPositionPercent(qreal position) { interface->call(__FUNCTION__, position); }
    void setVolumePercent(qreal volume)  { interface->call(__FUNCTION__, volume); }
private:
    QDBusInterface *interface;
};

#endif // MEDIAPLAYER_DBUS_H

