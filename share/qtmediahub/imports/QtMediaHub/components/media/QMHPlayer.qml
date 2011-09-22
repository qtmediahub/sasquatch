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
import QtMediaHub.components 1.0
import MediaModel 1.0
import VideoPlayer 1.0

Item {
    id: root

    anchors.fill: parent

    property bool shuffle: false
    property bool hasMedia: d.isValid && videoPlayer.source != ""
    //This reflects VideoItem perculiarities
    property bool playing: hasMedia && videoPlayer.playing && !paused

    property alias mediaElement: mediaElement
    property alias mediaInfo: currentMediaInfo
    property alias mediaPlaylist: playlist

    property alias videoPlayer: videoPlayer

    // ## Why are we exposing these properties when videoPlayer itself is exported?
    //These should all be bound to the backend
    // ## Given properties specific types other than variant
    property alias hasAudio: videoPlayer.hasAudio
    property alias hasVideo: videoPlayer.hasVideo
    property alias volume: videoPlayer.volume
    property alias position: videoPlayer.position
    property alias seekable: videoPlayer.seekable
    property alias status: videoPlayer.status
    property alias paused: videoPlayer.paused
    property alias playbackRate: videoPlayer.playbackRate
    property alias duration: videoPlayer.duration

    function stop() {
        videoPlayer.stop()
    }

    function play(mediaModel, row) {
        playlist.clear()
        playlist.addCurrentLevel(mediaModel)
        playIndex(mediaModel.dotDotPosition == MediaModel.Beginning ? row-1 /* adjust for dotdot */: row)
    }

    function playUri(uri) {
        videoPlayer.stop();
        videoPlayer.source = uri;
        videoPlayer.play();
    }

    function playNext() {
        playIndex(playlist.next())
    }

    function playPrevious() {
        playIndex(playlist.previous())
    }

    function playIndex(idx) {
        playlist.currentIndex = idx
        videoPlayer.stop();
        if (playlist.currentIndex == -1)
            return;
        videoPlayer.source = currentMediaInfo.getMetaData("uri", "file://")
        videoPlayer.play();
    }

    function increaseVolume() {
        videoPlayer.volume = (videoPlayer.volume + 0.05 > 1) ? 1.0 : videoPlayer.volume + 0.05
    }

    function decreaseVolume() {
        videoPlayer.volume = (videoPlayer.volume - 0.05 < 0) ? 0.0 : videoPlayer.volume - 0.05
    }

    function pause() {
        videoPlayer.paused = true
    }

    function resume() {
        videoPlayer.paused = false
    }

    function togglePlayPause() {
        if (!videoPlayer.playing || videoPlayer.paused) {
            videoPlayer.play()
        } else {
            videoPlayer.paused = true
        }
    }

    function seekForward() {
        if (videoPlayer.hasVideo)
            videoPlayer.position += 10000
        else
            videoPlayer.position += 1000
    }

    function seekBackward() {
        if (videoPlayer.hasVideo)
            videoPlayer.position -= 10000
        else
            videoPlayer.position -= 1000
    }

    QtObject {
        id: d
        property variant isValid: typeof videoPlayer != "undefined"
    }

    VideoPlayer {
        id: videoPlayer

        volume: runtime.config.value("media-volume", 0.5)
        onVolumeChanged: runtime.config.setValue("media-volume", volume)
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
        wrapAround: false
    }

    QMHMediaInfo {
        id: currentMediaInfo
    }

    Item {
        id: mediaElement
        x: 0; y: 0; width: parent.width; height: parent.height;
    }
}
