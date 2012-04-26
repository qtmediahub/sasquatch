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

import QtQuick 1.0
import Playlist 1.0
import QtMediaHub.components 1.0
import MediaModel 1.0
import MediaPlayer 1.0

Item {
    id: root

    anchors.fill: parent

    property bool shuffle: false
    property bool hasMedia: mediaPlayer.source != ""
    //This reflects VideoItem perculiarities
    property bool playing: hasMedia && mediaPlayer.playing && !paused

    property alias mediaPlaylist: playlist

    property alias hasAudio: mediaPlayer.hasAudio
    property alias hasVideo: mediaPlayer.hasVideo
    property alias volume: mediaPlayer.volume
    property alias position: mediaPlayer.position
    property alias seekable: mediaPlayer.seekable
    property alias status: mediaPlayer.status
    property alias paused: mediaPlayer.paused
    property alias playbackRate: mediaPlayer.playbackRate
    property alias duration: mediaPlayer.duration


    // metadata
    property string thumbnail: root.getThumbnail()
    property string artist: root.getMetaData("artist", qsTr("Unknown Artist"))
    property string album: root.getMetaData("album", qsTr("Unknown Album"))
    property string title: root.getMetaData("title", qsTr("Unknown Title"))
    property string track: root.getMetaData("track", "")
    property string mediaId: root.getMetaData("id", "0")


    // functions to retrieve metadata
    function getMetaData(role, defaultValue) {
        return playlist ? (playlist.data(playlist.currentIndex, role) || defaultValue) : ""
    }

    function getThumbnail() {
        if (playlist && playlist.currentIndex != -1) {
            var thumbnail = playlist.data(playlist.currentIndex, "previewUrl")
            if (thumbnail != "")
                return thumbnail;
        }
        return ""
    }


    // mediaplayer control functions
    function stop() {
        mediaPlayer.stop()
    }

    function play(mediaModel, row) {
        playlist.clear()
        playlist.addCurrentLevel(mediaModel)
        playIndex(mediaModel.dotDotPosition == MediaModel.Beginning ? row-1 /* adjust for dotdot */: row)
    }

    function playUri(uri) {
        mediaPlayer.stop();
        mediaPlayer.source = uri;
        mediaPlayer.play();
    }

    function playNext() {
        playIndex(playlist.next())
    }

    function playPrevious() {
        playIndex(playlist.previous())
    }

    function playIndex(idx) {
        playlist.currentIndex = idx
        mediaPlayer.stop();
        if (playlist.currentIndex == -1)
            return;
        mediaPlayer.source = root.getMetaData("uri", "file://")
        mediaPlayer.play();
    }

    function increaseVolume() {
        mediaPlayer.volume = (mediaPlayer.volume + 0.05 > 1) ? 1.0 : mediaPlayer.volume + 0.05
    }

    function decreaseVolume() {
        mediaPlayer.volume = (mediaPlayer.volume - 0.05 < 0) ? 0.0 : mediaPlayer.volume - 0.05
    }

    function pause() {
        mediaPlayer.paused = true
    }

    function resume() {
        mediaPlayer.paused = false
    }

    function togglePlayPause() {
        if (!mediaPlayer.playing || mediaPlayer.paused) {
            mediaPlayer.paused = false;
        } else {
            mediaPlayer.paused = true
        }
    }

    function seekForward() {
        if (mediaPlayer.hasVideo)
            mediaPlayer.position += 10000
        else
            mediaPlayer.position += 1000
    }

    function seekBackward() {
        if (mediaPlayer.hasVideo)
            mediaPlayer.position -= 10000
        else
            mediaPlayer.position -= 1000
    }

    MediaPlayer {
        id: mediaPlayer

        // TODO check if we override this binding
        volume: runtime.skin.settings.volume
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
}
