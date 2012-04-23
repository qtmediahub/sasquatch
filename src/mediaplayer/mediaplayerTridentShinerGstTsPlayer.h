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

#ifndef MEDIAPLAYER_TRIDENT_SHINER_GSTTSPLAYER_H
#define MEDIAPLAYER_TRIDENT_SHINER_GSTTSPLAYER_H

#include <QProcess>

#include "abstractmediaplayer.h"

class MediaPlayerTridentShinerGstTsPlayer : public AbstractMediaPlayer
{
    Q_OBJECT
public:
#ifdef QT5
    explicit MediaPlayerTridentShinerGstTsPlayer(QQuickItem *parent = 0);
#else
    explicit MediaPlayerTridentShinerGstTsPlayer(QDeclarativeItem *parent = 0);
#endif

    virtual QString source() const;
    virtual bool hasVideo() const;
    virtual bool hasAudio() const;
    virtual bool playing() const;
    virtual bool paused() const;

public slots:
    void setSource(const QString &uri);
    void stop();
    void pause();
    void resume();
    void play();
    void mute(bool on = true);
    void setPosition(int position);
    void setPositionPercent(qreal position);
    void setVolumePercent(qreal volume);

protected slots:
    void slotPlayerProcessError(QProcess::ProcessError error);
    void slotPlayerProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void slotPlayerProcessReadyReadStandardError();
    void slotPlayerProcessReadyReadStandardOutput();
    void slotPlayerProcessStarted();
    void slotPlayerProcessStateChanged(QProcess::ProcessState newState);

private:
    bool  m_playing;
    bool  m_hasVideo;
    bool  m_paused;

    QProcess *mp_player_process;

    QString  m_source_uri;
    QStringList  m_current_command_line_arguments;
};

#endif // MEDIAPLAYER_TRIDENT_SHINER_GSTTSPLAYER_H
