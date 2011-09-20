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


Item {
    id: root

    anchors.fill: parent

    property variant randomText: "OMGWTFBBQ"
    property variant mediaBackend

    property bool shuffle: false
    property bool hasMedia: d.isValid && mediaBackend.source != ""
    //This reflects VideoItem perculiarities
    property bool playing: hasMedia && mediaBackend.playing && !paused

    property alias mediaElement: mediaElement
    property alias mediaInfo: currentMediaInfo
    property alias mediaPlaylist: playlist

    //These should all be bound to the backend
    property variant hasAudio: false
    property variant hasVideo: false
    property variant volume: 0
    property variant position: 0
    property variant seekable: false
    property variant status: 0
    property variant paused: false
    property variant playbackRate: 1
    property variant duration: 0

    //This is the externally exposed media api
    //maps to backend logic of pluggable backends
    //dbus, QProcess, Video, etc

    function stop() {
        mediaBackend.stop()
    }

    function play(mediaModel, row) {
        playlist.clear()
        playlist.addCurrentLevel(mediaModel)
        playIndex(mediaModel.dotDotPosition == MediaModel.Beginning ? row-1 /* adjust for dotdot */: row)
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
        if (playlist.currentIndex == -1)
            return;
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
        mediaBackend.paused = true
    }

    function resume() {
        mediaBackend.paused = false
    }

    function togglePlayPause() {
        if (!mediaBackend.playing || mediaBackend.paused) {
            mediaBackend.play()
        } else {
            mediaBackend.paused = true
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

    QtObject {
        id: d
        property variant isValid: typeof mediaBackend != "undefined"
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

    QMHUtil {
       id: util
    }

    Component.onCompleted: {
        mediaBackend = util.createQmlObjectFromFile(runtime.config.value("overlay-mode", false) ? "media/QMHExternalBackend.qml" : "media/QMHVideoItemBackend.qml", {}, mediaElement)
        if (mediaBackend) {
            //out
            util.createBinding("root", "hasAudio", "mediaBackend.hasAudio", mediaBackend)
            util.createBinding("root", "hasVideo", "mediaBackend.hasVideo", mediaBackend)
            util.createBinding("root", "seekable", "mediaBackend.seekable", mediaBackend)
            util.createBinding("root", "status",   "mediaBackend.status",   mediaBackend)
            util.createBinding("root", "duration", "mediaBackend.duration", mediaBackend)

            //bidirectional
            util.createBinding("root", "playing", "mediaBackend.playing", mediaBackend)
            util.createBinding("root", "volume", "mediaBackend.volume", mediaBackend)
            util.createBinding("root", "position", "mediaBackend.position", mediaBackend)
            util.createBinding("root", "paused", "mediaBackend.paused", mediaBackend)
            util.createBinding("root", "playbackRate", "mediaBackend.playbackRate", mediaBackend)

            util.createBinding("mediaBackend", "playing", "root.playing", mediaBackend)
            util.createBinding("mediaBackend", "volume", "root.volume", mediaBackend)
            util.createBinding("mediaBackend", "position", "root.position", mediaBackend)
            util.createBinding("mediaBackend", "paused", "root.paused", mediaBackend)
            util.createBinding("mediaBackend", "playbackRate", "root.playbackRate", mediaBackend)
        }
    }
}
