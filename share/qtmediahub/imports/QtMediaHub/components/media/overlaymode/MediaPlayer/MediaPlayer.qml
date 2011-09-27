import QtQuick 1.0
import AbstractMediaPlayer 1.0
import QtMediaHub.components 1.0
import OverlayModeMediaPlayer 1.0

Item {
    id: root
    anchors.fill: parent

    OverlayModeMediaPlayer {
        id: mediaPlayer
        anchors.fill: parent
    }

    property alias source: mediaPlayer.source
    property alias playing: mediaPlayer.playing
    property alias hasAudio: mediaPlayer.hasAudio
    property alias hasVideo: mediaPlayer.hasVideo
    property alias volume: mediaPlayer.volume

    property alias position: mediaPlayer.position
    property alias seekable: mediaPlayer.seekable
    property alias status: mediaPlayer.status
    property alias paused: mediaPlayer.paused
    property alias playbackRate: mediaPlayer.playbackRate
    property alias duration: mediaPlayer.duration

    function play() { mediaPlayer.play() }
    function stop() { mediaPlayer.stop() }
    function pause() { mediaPlayer.pause() }
    function resume() { mediaPlayer.resume() }
}
