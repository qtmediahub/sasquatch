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

Item {
    id: root

    property variant mediaItem
    property alias mediaPlaylist: playlist

    Playlist {
        id: playlist
        playMode: Playlist.Normal
    }

    function stop() {
        mediaItem.stop()
    }

    function play(mediaModel, row) {
        playlist.clear()
        playlist.addCurrentLevel(mediaModel)
        playIndex(row-1 /* adjust for dotdot */)
    }

    function playUri(uri) {
        mediaItem.stop();
        mediaItem.source = uri;
        mediaItem.play();
    }

    function playNext() {
        playIndex(playlist.next())
    }

    function playPrevious() {
        playIndex(playlist.previous())
    }

    function playIndex(idx) {
        playlist.currentIndex = idx
        mediaItem.stop();
        mediaItem.source = getMetaData("uri", "file://")
        mediaItem.play();
    }

    function increaseVolume() {
        mediaItem.volume = (mediaItem.volume + 0.05 > 1) ? 1.0 : mediaItem.volume + 0.05
    }

    function decreaseVolume() {
        mediaItem.volume = (mediaItem.volume - 0.05 < 0) ? 0.0 : mediaItem.volume - 0.05
    }

    function pause() {
        mediaItem.pause()
    }

    function resume() {
        mediaItem.play()
    }

    function togglePlayPause() {
        if (!mediaItem.playing || mediaItem.paused) {
            mediaItem.play()
        } else {
            mediaItem.pause()
        }
    }

    function seekForward()
    {
        if (mediaItem.hasVideo)
            mediaItem.position += 10000
        else
            mediaItem.position += 1000
    }

    function seekBackward()
    {
        if (mediaItem.hasVideo)
            mediaItem.position -= 10000
        else
            mediaItem.position -= 1000
    }

    function getMetaData(role, defaultValue) {
        return playlist ? (playlist.data(playlist.currentIndex, role) || defaultValue) : ""
    }

    function getThumbnail(defaultAudio, defaultVideo) {
        if (playlist && playlist.currentIndex != -1) {
            var thumbnail = playlist.data(playlist.currentIndex, "previewUrl")
            if (thumbnail != "")
                return thumbnail;
        }
        return mediaItem.hasVideo ? defaultVideo : defaultAudio;
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

    Component.onCompleted: {
        //Just for testing
        if (false) {
            runtime.mediaBackendInterface.play()
            runtime.mediaBackendInterface.mute(true)
            runtime.mediaBackendInterface.mute(false)
            runtime.mediaBackendInterface.loadUri("file:///wickedpath")
        }
    }
}
