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

#include "mediaplayerST7540.h"

#define QMH_DEBUG if (0) qDebug() << __PRETTY_FUNCTION__ << "::" << __LINE__
#define QMH_WARNING qWarning() << __PRETTY_FUNCTION__ << "::" << __LINE__

#ifdef SCENEGRAPH
MediaPlayerST7540::MediaPlayerST7540(QQuickItem *parent) :
#else
MediaPlayerST7540::MediaPlayerST7540(QDeclarativeItem *parent) :
#endif
    AbstractMediaPlayer(parent),
    m_playing(false),
    m_hasVideo(false),
    m_paused(false),
    mp_player_process(NULL),
    m_source_uri(QString()),
    st_current_command_line_arguments(QString()),
    mp_player_feedback_file(NULL),
    mp_player_feedback_socket_notifier(NULL),
    mp_player_feedback_textstream(NULL),
    m_status_check_timer_id(0)

{
    QMH_DEBUG;

    mp_player_process = new QProcess(this);

    if (NULL != mp_player_process) {
        QMH_DEBUG;
        connect(mp_player_process, SIGNAL(error(QProcess::ProcessError)), this,  SLOT(slotPlayerProcessError(QProcess::ProcessError)));
        connect(mp_player_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,  SLOT(slotPlayerProcessFinished(int, QProcess::ExitStatus)));
        connect(mp_player_process, SIGNAL(readyReadStandardError()), this,  SLOT(slotPlayerProcessReadyReadStandardError()));
        connect(mp_player_process, SIGNAL(readyReadStandardOutput()), this,  SLOT(slotPlayerProcessReadyReadStandardOutput()));
        connect(mp_player_process, SIGNAL(started()), this,  SLOT(slotPlayerProcessStarted()));
        connect(mp_player_process, SIGNAL(stateChanged(QProcess::ProcessState)), this,  SLOT(slotPlayerProcessStateChanged(QProcess::ProcessState)));
        QMH_DEBUG;
    }

    mp_player_feedback_file = new QFile("/tmp/stapp_log", this);

    if (NULL != mp_player_feedback_file) {
        QMH_DEBUG;
        if (mp_player_feedback_file->open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMH_DEBUG;
            mp_player_feedback_socket_notifier = new QSocketNotifier(mp_player_feedback_file->handle(), QSocketNotifier::Read, this);
            QMH_DEBUG;
            if (NULL != mp_player_feedback_socket_notifier) {
                QMH_DEBUG;
                connect(mp_player_feedback_socket_notifier, SIGNAL(activated(int)), this,  SLOT(slotPlayerFeedBackSocketNotifierActivated(int)));
                QMH_DEBUG;
                mp_player_feedback_textstream = new QTextStream(mp_player_feedback_file);
                if (NULL != mp_player_feedback_textstream) {
                    QMH_DEBUG;
                }
                QMH_DEBUG;
            }
            QMH_DEBUG;
        }
        QMH_DEBUG;
    }
    QMH_DEBUG;

    m_status_check_timer_id = startTimer(5000);

    QMH_DEBUG;
}


void MediaPlayerST7540::setSource(const QString &uri)
{
    m_source_uri = uri;

    QMH_DEBUG << ":" << m_source_uri;
    st_current_command_line_arguments = QString();

    QString filename = m_source_uri;
    filename.remove(QString("file://"));
    QMH_DEBUG << ":" << filename << ":" << filename.endsWith("mp3");
    if(filename.endsWith("mp3")) {
        st_current_command_line_arguments.append("echo \'playmedia \"");
        st_current_command_line_arguments.append(filename);
        st_current_command_line_arguments.append("\" \' > /tmp/stapp_fifo");
    } else {
        st_current_command_line_arguments.append("echo \'playmedia \"");
        st_current_command_line_arguments.append(filename);
        st_current_command_line_arguments.append("\" \' > /tmp/stapp_fifo");
    }
    emit sourceChanged();
    QMH_DEBUG << ":" << st_current_command_line_arguments;
}


void MediaPlayerST7540::stop()
{
    QMH_DEBUG;
    if (true == m_playing) {
        QMH_DEBUG;
        system("echo 'PLAYER_STOP 0' > /tmp/stapp_fifo");
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


void MediaPlayerST7540::pause()
{
    QMH_DEBUG;
    if (false == m_paused) {
        QMH_DEBUG;
        system("echo 'PLAYER_PAUSE 0' > /tmp/stapp_fifo");
        QMH_DEBUG;
        m_paused = true;
        QMH_DEBUG << "m_paused : " << m_paused;
        emit pausedChanged();
        QMH_DEBUG;
    }
    QMH_DEBUG;
}


void MediaPlayerST7540::resume()
{
    QMH_DEBUG;
    if (true == m_paused) {
        QMH_DEBUG;
        system("echo 'PLAYER_RESUME 0' > /tmp/stapp_fifo");
        QMH_DEBUG;
        m_paused = false;
        QMH_DEBUG << "m_paused : " << m_paused;
        emit playingChanged();
        QMH_DEBUG;
    }
    QMH_DEBUG;
}


void MediaPlayerST7540::play()
{
    QMH_DEBUG;
    if (false == m_playing) {
        QMH_DEBUG;
        system(st_current_command_line_arguments.toUtf8().constData());
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
    QMH_DEBUG;
}


void MediaPlayerST7540::mute(bool on)
{
    QMH_DEBUG << ":" << on;
}


void MediaPlayerST7540::setPosition(int position)
{
    QMH_DEBUG << ":" << position;
}


void MediaPlayerST7540::setPositionPercent(qreal position)
{
    QMH_DEBUG << ":" << position;
}


void MediaPlayerST7540::setVolumePercent(qreal volume)
{
    QMH_DEBUG << ":" << volume;
}


QString MediaPlayerST7540::source() const
{
    QMH_DEBUG << "m_source_uri : " << m_source_uri;
    return m_source_uri;
}


bool MediaPlayerST7540::hasAudio() const
{
    QMH_DEBUG;
    return false;
}


bool MediaPlayerST7540::hasVideo() const
{
    QMH_DEBUG << "m_hasVideo : " << m_hasVideo;
    return m_hasVideo;
}


bool MediaPlayerST7540::playing() const
{
    QMH_DEBUG << "m_playing : " << m_playing;
    return m_playing;
}


bool MediaPlayerST7540::paused() const
{
    QMH_DEBUG << "m_paused : " << m_paused;
    return m_paused;
}

void MediaPlayerST7540::slotPlayerProcessError(QProcess::ProcessError error)
{
    QMH_DEBUG << "error : " << error;
}

void MediaPlayerST7540::slotPlayerProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QMH_DEBUG << "exitCode : " << exitCode << "exitStatus : " << exitStatus;
}


void MediaPlayerST7540::slotPlayerProcessReadyReadStandardError()
{
    QMH_DEBUG;
}


void MediaPlayerST7540::slotPlayerProcessReadyReadStandardOutput()
{
    QMH_DEBUG;
}


void MediaPlayerST7540::slotPlayerProcessStarted()
{
    QMH_DEBUG;
}


void MediaPlayerST7540::slotPlayerProcessStateChanged(QProcess::ProcessState newState)
{
    QMH_DEBUG << "newState : " << newState;
}


void MediaPlayerST7540::slotPlayerFeedBackSocketNotifierActivated(int socket)
{
//BAS_TEMP    QMH_DEBUG << "socket : " << socket;

/*
    QFile tempFile(this);

    if (tempFile.open( socket, QIODevice::ReadOnly | QIODevice::Text, QFile::DontCloseHandle)) {
//BAS_TEMP    if (tempFile.open( socket, QIODevice::ReadOnly | QIODevice::Text)) {
        QMH_DEBUG;
    } else {
        QMH_DEBUG;
        return ;
    }

    QTextStream in(&tempFile);
*/

/*
    QTextStream in(mp_player_feedback_file);

    while (!in.atEnd()) {
        QString line = in.readLine();
        QMH_DEBUG << "line : " << line;
    }
*/

    while (!mp_player_feedback_textstream->atEnd()) {
        QString line = mp_player_feedback_textstream->readLine();
        QMH_DEBUG << "line : " << line;

        if (line.contains("STMP_EVENT_PLAY_ENDED")) {
            QMH_DEBUG;
            AbstractMediaPlayer::setStatus(AbstractMediaPlayer::EndOfMedia);
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
    }

//BAS_TEMP    QMH_DEBUG;
}


void MediaPlayerST7540::timerEvent(QTimerEvent *event)
{
//BAS_TEMP    QMH_DEBUG << "Timer ID:" << event->timerId();
    if (m_status_check_timer_id == event->timerId()) {
        if (true == m_playing && true != m_paused ) {
//BAS_TEMP            QMH_DEBUG;
//BAS_TEMP            system("echo 'PLAYER_PRINTSTATE 0' > /tmp/stapp_fifo");
//BAS_TEMP
            system("echo '\n' > /tmp/stapp_fifo");
//BAS_TEMP            QMH_DEBUG;
        }
    }
//BAS_TEMP    QMH_DEBUG;
}
