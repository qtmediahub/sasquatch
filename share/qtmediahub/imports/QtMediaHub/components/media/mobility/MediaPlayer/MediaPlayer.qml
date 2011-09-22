import QtQuick 1.0
import QtMultimediaKit 1.1

Item {
    property alias source: video.source
    property alias playing: video.playing
    property alias hasAudio: video.hasAudio
    property alias hasVideo: video.hasVideo
    property alias volume: video.volume
    property alias position: video.position
    property alias seekable: video.seekable
    property alias status: video.status
    property alias paused: video.paused
    property alias playbackRate: video.playbackRate
    property alias duration: video.duration

    anchors.fill: parent

    function play() { video.play() }
    function stop() { video.stop() }
    function pause() { video.pause() }
    function resume() { video.resume() }

    Video {
        id: video
        anchors.fill: parent

        //Work around VideoItem shortcomings
        property int _seekPos : -1

        onSeekableChanged : {
            if (seekable && _seekPos != -1) {
                position = _seekPos
                _seekPos = -1
            }
        }

        function seek(pos) {
            //This works? what happens when already seekable?
            _seekPos = pos
        }
    }
}

