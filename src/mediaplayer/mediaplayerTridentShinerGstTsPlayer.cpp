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

#include "mediaplayerTridentShinerGstTsPlayer.h"

#define QMH_DEBUG if (0) qDebug() << __PRETTY_FUNCTION__ << ":" << __LINE__
#define QMH_WARNING qWarning() << __PRETTY_FUNCTION__ << ":" << __LINE__

#ifdef QT5
MediaPlayerTridentShinerGstTsPlayer::MediaPlayerTridentShinerGstTsPlayer(QQuickItem *parent) :
#else
MediaPlayerTridentShinerGstTsPlayer::MediaPlayerTridentShinerGstTsPlayer(QDeclarativeItem *parent) :
#endif
    AbstractMediaPlayer(parent),
    m_playing(false),
    m_hasVideo(false),
    m_paused(false),
    mp_player_process(NULL),
    m_source_uri(QString()),
    m_current_command_line_arguments(QStringList())
{
    QMH_DEBUG;

    mp_player_process = new QProcess(this);

    if (NULL != mp_player_process) {
        QMH_DEBUG;
        connect(mp_player_process, SIGNAL(error(QProcess::ProcessError)), this,  SLOT(slotPlayerProcessError(QProcess::ProcessError)));
        connect(mp_player_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,  SLOT(slotPlayerProcessFinished(int, QProcess::ExitStatus)));
//BAS_TEMP        connect(mp_player_process, SIGNAL(readyReadStandardError()), this,  SLOT(slotPlayerProcessReadyReadStandardError()));
//BAS_TEMP        connect(mp_player_process, SIGNAL(readyReadStandardOutput()), this,  SLOT(slotPlayerProcessReadyReadStandardOutput()));
        connect(mp_player_process, SIGNAL(started()), this,  SLOT(slotPlayerProcessStarted()));
        connect(mp_player_process, SIGNAL(stateChanged(QProcess::ProcessState)), this,  SLOT(slotPlayerProcessStateChanged(QProcess::ProcessState)));
        QMH_DEBUG;
    }

    QMH_DEBUG;
}


void MediaPlayerTridentShinerGstTsPlayer::setSource(const QString &uri)
{
    m_source_uri = uri;

    QMH_DEBUG << ":" << m_source_uri;
    m_current_command_line_arguments = QStringList();

    QString filename = m_source_uri;
    filename.remove(QString("file://"));
    m_current_command_line_arguments.append(filename);
    QMH_DEBUG << ":" << m_current_command_line_arguments;
    emit sourceChanged();
    QMH_DEBUG;
}


void MediaPlayerTridentShinerGstTsPlayer::stop()
{
/*
    QMH_DEBUG;
    if ( QProcess::Running == mp_player_process->state() ) {
        QMH_DEBUG;
        if (true == m_playing) {
            QMH_DEBUG;
            QString command = QString("e\n");
            mp_player_process->write(qPrintable(command), qstrlen(qPrintable(command)));
            QMH_DEBUG;
            mp_player_process->waitForBytesWritten(-1);
            QMH_DEBUG;
            m_current_command_line_arguments = QStringList();
            QMH_DEBUG;
            //BAS_TEMP	AbstractMediaPlayer::setStatus(AbstractMediaPlayer::EndOfMedia);
            QMH_DEBUG;
            m_hasVideo = false;
            QMH_DEBUG << "m_hasVideo : " << m_hasVideo;
            //BAS_TEMP
            emit hasVideoChanged();
            QMH_DEBUG;
            m_playing = false;
            QMH_DEBUG << "m_playing : " << m_playing;
            //BAS_TEMP
            emit playingChanged();
            QMH_DEBUG;
        }
        QMH_DEBUG;
    }
*/
    QMH_DEBUG;
}


void MediaPlayerTridentShinerGstTsPlayer::pause()
{
/*
    QMH_DEBUG;
    if ( QProcess::Running == mp_player_process->state() ) {
        QMH_DEBUG;
        if (false == m_paused) {
            QMH_DEBUG;
            QString command = QString("p\n");
            mp_player_process->write(qPrintable(command), qstrlen(qPrintable(command)));
            QMH_DEBUG;
            mp_player_process->waitForBytesWritten(-1);
            QMH_DEBUG;
            m_paused = true;
            QMH_DEBUG << "m_paused : " << m_paused;
            emit pausedChanged();
            QMH_DEBUG;
        }
        QMH_DEBUG;
    }
*/
    QMH_DEBUG;
}


void MediaPlayerTridentShinerGstTsPlayer::resume()
{
/*
    QMH_DEBUG;
    if ( QProcess::Running == mp_player_process->state() ) {
        QMH_DEBUG;
        if (true == m_paused) {
            QMH_DEBUG;
            QString command = QString("r\n");
            mp_player_process->write(qPrintable(command), qstrlen(qPrintable(command)));
            QMH_DEBUG;
            mp_player_process->waitForBytesWritten(-1);
            QMH_DEBUG;
            m_paused = false;
            QMH_DEBUG << "m_paused : " << m_paused;
            emit pausedChanged();
            QMH_DEBUG;
        }
        QMH_DEBUG;
    }
*/
    QMH_DEBUG;
}


void MediaPlayerTridentShinerGstTsPlayer::play()
{
    QMH_DEBUG;
    if ( QProcess::NotRunning == mp_player_process->state() ) {
        QMH_DEBUG;
        if (false == m_playing) {
            QMH_DEBUG << ":" << m_current_command_line_arguments;
            mp_player_process->start( QString("/opt/bin/GstTsPlayer.02"), m_current_command_line_arguments);
            QMH_DEBUG;
            AbstractMediaPlayer::setStatus(AbstractMediaPlayer::Loaded);
            QMH_DEBUG;
            m_hasVideo = true;
            QMH_DEBUG << "m_hasVideo : " << m_hasVideo;
            emit hasVideoChanged();
            QMH_DEBUG;
            m_playing = true;
            QMH_DEBUG << "m_playing : " << m_playing;
            emit playingChanged();
            QMH_DEBUG;
        }
        QMH_DEBUG << ":" << m_current_command_line_arguments;
    }
    QMH_DEBUG;
}


void MediaPlayerTridentShinerGstTsPlayer::mute(bool on)
{
    QMH_DEBUG << ":" << on;
}


void MediaPlayerTridentShinerGstTsPlayer::setPosition(int position)
{
    QMH_DEBUG << ":" << position;
}


void MediaPlayerTridentShinerGstTsPlayer::setPositionPercent(qreal position)
{
    QMH_DEBUG << ":" << position;
}


void MediaPlayerTridentShinerGstTsPlayer::setVolumePercent(qreal volume)
{
    QMH_DEBUG << ":" << volume;
}


QString MediaPlayerTridentShinerGstTsPlayer::source() const
{
    QMH_DEBUG << "m_source_uri : " << m_source_uri;
    return m_source_uri;
}


bool MediaPlayerTridentShinerGstTsPlayer::hasAudio() const
{
    QMH_DEBUG;
    return false;
}


bool MediaPlayerTridentShinerGstTsPlayer::hasVideo() const
{
    QMH_DEBUG << "m_hasVideo : " << m_hasVideo;
    return m_hasVideo;
}


bool MediaPlayerTridentShinerGstTsPlayer::playing() const
{
    QMH_DEBUG << "m_playing : " << m_playing;
    return m_playing;
}


bool MediaPlayerTridentShinerGstTsPlayer::paused() const
{
    QMH_DEBUG << "m_paused : " << m_paused;
    return m_paused;
}

void MediaPlayerTridentShinerGstTsPlayer::slotPlayerProcessError(QProcess::ProcessError error)
{
    QMH_DEBUG << "error : " << error;
    emit sourceChanged();
    QMH_DEBUG;
    emit statusChanged();
    QMH_DEBUG;
    emit hasAudioChanged();
    QMH_DEBUG;
    emit hasVideoChanged();
    QMH_DEBUG;
    emit playingChanged();
    QMH_DEBUG;
    emit volumeChanged();
    QMH_DEBUG;
    emit positionChanged();
    QMH_DEBUG;
    emit seekableChanged();
    QMH_DEBUG;
    emit pausedChanged();
    QMH_DEBUG;
    emit playbackRateChanged();
    QMH_DEBUG;
    emit durationChanged();
    QMH_DEBUG;
}

void MediaPlayerTridentShinerGstTsPlayer::slotPlayerProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QMH_DEBUG << "exitCode : " << exitCode << "exitStatus : " << exitStatus;
    AbstractMediaPlayer::setStatus(AbstractMediaPlayer::EndOfMedia);
    m_hasVideo = false;
    QMH_DEBUG << "m_hasVideo : " << m_hasVideo;
    //BAS_TEMP
    emit hasVideoChanged();
    QMH_DEBUG;
    m_playing = false;
    QMH_DEBUG << "m_playing : " << m_playing;
    //BAS_TEMP
    emit playingChanged();
    QMH_DEBUG;
}

void MediaPlayerTridentShinerGstTsPlayer::slotPlayerProcessReadyReadStandardError()
{
    QMH_DEBUG;
    if ( QProcess::Running == mp_player_process->state() ) {
        QMH_DEBUG;
        QByteArray stdErrorData;
        stdErrorData = mp_player_process->readAllStandardError();
        QMH_DEBUG << ":" << stdErrorData;
    }
    QMH_DEBUG;
}

void MediaPlayerTridentShinerGstTsPlayer::slotPlayerProcessReadyReadStandardOutput()
{
    QMH_DEBUG;
    if ( QProcess::Running == mp_player_process->state() ) {
        QMH_DEBUG;
        QTextStream in(mp_player_process);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QMH_DEBUG << ": line : " << line;
        }
    }
    QMH_DEBUG;
}


void MediaPlayerTridentShinerGstTsPlayer::slotPlayerProcessStarted()
{
    QMH_DEBUG;
    emit sourceChanged();
    QMH_DEBUG;
    emit statusChanged();
    QMH_DEBUG;
    emit hasAudioChanged();
    QMH_DEBUG;
    emit hasVideoChanged();
    QMH_DEBUG;
    emit playingChanged();
    QMH_DEBUG;
    emit volumeChanged();
    QMH_DEBUG;
    emit positionChanged();
    QMH_DEBUG;
    emit seekableChanged();
    QMH_DEBUG;
    emit pausedChanged();
    QMH_DEBUG;
    emit playbackRateChanged();
    QMH_DEBUG;
    emit durationChanged();
    QMH_DEBUG;
}


void MediaPlayerTridentShinerGstTsPlayer::slotPlayerProcessStateChanged(QProcess::ProcessState newState)
{
    QMH_DEBUG << "newState : " << newState;
    emit sourceChanged();
    QMH_DEBUG;
    emit statusChanged();
    QMH_DEBUG;
    emit hasAudioChanged();
    QMH_DEBUG;
    emit hasVideoChanged();
    QMH_DEBUG;
    emit playingChanged();
    QMH_DEBUG;
    emit volumeChanged();
    QMH_DEBUG;
    emit positionChanged();
    QMH_DEBUG;
    emit seekableChanged();
    QMH_DEBUG;
    emit pausedChanged();
    QMH_DEBUG;
    emit playbackRateChanged();
    QMH_DEBUG;
    emit durationChanged();
    QMH_DEBUG;
}
