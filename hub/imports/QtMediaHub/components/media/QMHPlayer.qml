/****************************************************************************

This file is part of the QtMediaHub project on http://www.gitorious.org.

Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Nokia Corporation (qt-info@nokia.com)**

You may use this file under the terms of the BSD license as follows:

"Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."

****************************************************************************/

import QtQuick 1.0
import Playlist 1.0
import QtMultimediaKit 1.1

Item {
    id: root

    anchors.fill: parent

    property bool shuffle: false
    property bool hasMedia: !!mediaBackend && mediaBackend.source != ""
    //This reflects VideoItem perculiarities
    property bool playing: hasMedia && mediaBackend.playing && !paused

    property alias mediaElement: mediaBackend
    property alias mediaInfo: currentMediaInfo
    property alias mediaPlaylist: playlist

    property alias hasAudio: mediaBackend.hasAudio
    property alias hasVideo: mediaBackend.hasVideo
    property alias volume: mediaBackend.volume
    property alias position: mediaBackend.position
    property alias seekable: mediaBackend.seekable
    property alias status: mediaBackend.status
    property alias paused: mediaBackend.paused
    property alias playbackRate: mediaBackend.playbackRate

    //This is the externally exposed media api
    //maps to backend logic of pluggable backends
    //dbus, QProcess, Video, etc

    function stop() {
        mediaBackend.stop()
    }

    function play(mediaModel, row) {
        playlist.clear()
        playlist.addCurrentLevel(mediaModel)
        playIndex(row-1 /* adjust for dotdot */)
    }

    function playUri(uri) {
        mediaBackend.stop();
        mediaBackend.source = uri;
        mediaBackend.play();
    }

    function playNext() {
        playIndex(playlist.next())
    }

    function playPrevious() {
        playIndex(playlist.previous())
    }

    function playIndex(idx) {
        playlist.currentIndex = idx
        mediaBackend.stop();
        mediaBackend.source = currentMediaInfo.getMetaData("uri", "file://")
        mediaBackend.play();
    }

    function increaseVolume() {
        mediaBackend.volume = (mediaBackend.volume + 0.05 > 1) ? 1.0 : mediaBackend.volume + 0.05
    }

    function decreaseVolume() {
        mediaBackend.volume = (mediaBackend.volume - 0.05 < 0) ? 0.0 : mediaBackend.volume - 0.05
    }

    function pause() {
        mediaBackend.pause()
    }

    function resume() {
        mediaBackend.play()
    }

    function togglePlayPause() {
        if (!mediaBackend.playing || mediaBackend.paused) {
            mediaBackend.play()
        } else {
            mediaBackend.pause()
        }
    }

    function seekForward()
    {
        if (mediaBackend.hasVideo)
            mediaBackend.position += 10000
        else
            mediaBackend.position += 1000
    }

    function seekBackward()
    {
        if (mediaBackend.hasVideo)
            mediaBackend.position -= 10000
        else
            mediaBackend.position -= 1000
    }

    // RPC requests
    Connections {
        target: runtime.mediaPlayerRpc
        onStopRequested: root.stop()
        onPauseRequested: root.pause()
        onResumeRequested: root.resume()
        onTogglePlayPauseRequested: root.togglePlayPause()
        onNextRequested: root.playNext()
        onPreviousRequested: root.playPrevious()
        onVolumeUpRequested: root.increaseVolume()
        onVolumeDownRequested: root.decreaseVolume()
        onPlayRemoteSourceRequested: root.playUri(uri)
    }

    Playlist {
        id: playlist
        playMode: root.shuffle ? Playlist.Shuffle : Playlist.Normal
    }

    QMHMediaInfo {
        id: currentMediaInfo
    }

    Video {
        //This is one backend and needs to be pluggable
        id: mediaBackend
        x: 0; y: 0; width: parent.width; height: parent.height; z: 1

        visible: hasVideo
        volume: runtime.config.value("media-volume", 0.1)

        //Work around VideoItem shortcomings
        property int _seekPos : -1

        onSeekableChanged : {
            if (seekable && _seekPos != -1) {
                position = _seekPos
                _seekPos = -1
            }
        }

        function seek(pos) {
            _seekPos = pos
        }
    }
}
