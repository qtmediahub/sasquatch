/****************************************************************************

This file is part of the QtMediaHub project on http://www.qtmediahub.com

Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Girish Ramakrishnan girish@forwardbias.in
Contact:  Nokia Corporation donald.carr@nokia.com
Contact:  Nokia Corporation johannes.zellner@nokia.com
Contact:  Shrikant Dhumal shrikant_dhumal@persistent.co.in / ext-shrikant.dhumal@nokia.com

You may use this file under the terms of the BSD license as follows:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include "mediaplayer7425.h"

#define NLLR "\r"

#ifdef QT5
MediaPlayer7425::MediaPlayer7425(QQuickItem *parent) :
#else
MediaPlayer7425::MediaPlayer7425(QDeclarativeItem *parent) :
#endif
    AbstractMediaPlayer(parent),
    m_playing(false),
    m_hasVideo(false),
    m_paused(false),
    m_cmdLineArgs(QString()),
	m_pPlayer(NULL)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << NLLR;

    m_pPlayer = new QProcess(this);
    connect(m_pPlayer, SIGNAL(finished( int, QProcess::ExitStatus)), this, SLOT(slotProcessFinished(int, QProcess::ExitStatus)));

    connect(m_pPlayer, SIGNAL(stateChanged( QProcess::ProcessState)), this, SLOT(slotStateChanged(QProcess::ProcessState)));
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__;

    m_cmdLineArgs.clear();
}

void MediaPlayer7425::setSource(const QString &uri)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__;

    QString filename = uri;
    filename.remove(QString("file://"));
    m_cmdLineArgs.clear();
    m_cmdLineArgs.append(filename);

    emit sourceChanged();
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << ":" << m_cmdLineArgs;
}

void MediaPlayer7425::stop()
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__;
    if (true == m_playing)
    {
        int rbytes;
        rbytes = m_pPlayer->write("q\n");
        m_pPlayer->waitForBytesWritten(-1);
		m_hasVideo = false;
		emit hasVideoChanged();
		m_playing = false;
		emit playingChanged();
        qDebug() << "quit/stop pressed" << rbytes;
    }
}

void MediaPlayer7425::pause()
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__;
    if (false == m_paused)
    {
        int rbytes;
        rbytes = m_pPlayer->write("pause\n");
        m_pPlayer->waitForBytesWritten(-1);
        m_paused = true;
		emit pausedChanged();
        qDebug() << "pause pressed" << rbytes;
    }
}

void MediaPlayer7425::resume()
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__;
    if (true == m_paused)
    {
        int rbytes;
        rbytes = m_pPlayer->write("play\n");
        m_pPlayer->waitForBytesWritten(-1);
        m_paused = false;
		emit playingChanged();
        qDebug() << "play/resume pressed" << rbytes;
    }
}

void MediaPlayer7425::play()
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__;
    if ( false == m_playing )
    {
        //m_cmdLineArgs.append(QString(" -get_command"));
        //m_cmdLineArgs.append(QString(" 1"));

        m_pPlayer->start( QString("/usr/local/bin/gstreamer/gplayback/gplayback"), m_cmdLineArgs);
        AbstractMediaPlayer::setStatus(AbstractMediaPlayer::Loaded);
        m_hasVideo = true;
        emit hasVideoChanged();
        m_playing = true;
        emit playingChanged();

        qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__;
    }
}


void MediaPlayer7425::mute(bool on)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << ":" << on;
}


void MediaPlayer7425::setPosition(int position)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << ":" << position;
}


void MediaPlayer7425::setPositionPercent(qreal position)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << ":" << position;
}


void MediaPlayer7425::setVolumePercent(qreal volume)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << ":" << volume;
}


QString MediaPlayer7425::source() const
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << "m_cmdLineArgs : " << m_cmdLineArgs;
    
    return m_cmdLineArgs.isEmpty()?QString():m_cmdLineArgs.at(0);
}


bool MediaPlayer7425::hasAudio() const
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__;
    return false;
}


bool MediaPlayer7425::hasVideo() const
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << "m_hasVideo : " << m_hasVideo;
    return m_hasVideo;
}


bool MediaPlayer7425::playing() const
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << "m_playing : " << m_playing;
    return m_playing;
}


bool MediaPlayer7425::paused() const
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << "m_paused : " << m_paused;
    return m_paused;
}

MediaPlayer7425::~MediaPlayer7425()
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__;

    AbstractMediaPlayer::setStatus(AbstractMediaPlayer::EndOfMedia);
    delete m_pPlayer;
    m_pPlayer = NULL;
    m_cmdLineArgs.clear();
}

void MediaPlayer7425::slotProcessFinished( int exitCode, QProcess::ExitStatus exitStatus)
{
    AbstractMediaPlayer::setStatus(AbstractMediaPlayer::EndOfMedia);
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << ":" << exitCode << ":" << exitStatus;
}

void MediaPlayer7425::slotStateChanged(QProcess::ProcessState newState)
{
    AbstractMediaPlayer::playingChanged();
    qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__ << ":" << newState;
}
